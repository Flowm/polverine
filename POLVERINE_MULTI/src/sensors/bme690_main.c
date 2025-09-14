/*!
 * @file bsec_iot_example.c
 *
 * @brief
 * Example for using of BSEC library in a fixed configuration with the BME68x sensor.
 * This works by running an endless loop in the bsec_iot_loop() function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bme690_io.h"
#include "bsec_iaq.h"
#include "bsec_integration.h"
#include "led_control.h"
#include "nvs.h"
#include "polverine_cfg.h"
#include "sensor_buffer.h"
#include "sensor_data_broker.h"

static const char *TAG = "bme690";

#define SID_BME69X    UINT16_C(0x093)
#define SID_BME69X_X8 UINT16_C(0x057)

// NVS storage constants for BME690 state
#define BME690_NVS_NAMESPACE        "bme690"
#define BME690_NVS_STATE_KEY        "bsec_state"
#define BME690_STATE_SAVE_PERIOD_MS 3600000 // Save state every hour

static uint8_t dev_addr[NUM_OF_SENS];
uint8_t bme_cs[NUM_OF_SENS];
extern uint8_t n_sensors;
extern uint8_t *bsecInstance[NUM_OF_SENS];

extern char shortId[7];

/* Callback methods */

static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer);

static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer);

static uint32_t get_timestamp_ms();

static void state_save(const uint8_t *state_buffer, uint32_t length);

static void output_ready(outputs_t *output);

static bme690_sensor_buffer_t sensor_buffer;
static uint32_t startup_time = 0;
static bool first_output = true;

void bme690_task(void *) {
    bme690_i2c_init();

    bme690_buffer_init(&sensor_buffer);

    bsec_version_t version;
    return_values_init ret = {BME69X_OK, BSEC_OK};

    ret = bsec_iot_init(SAMPLE_RATE, bme69x_interface_init, state_load, config_load);

    ESP_LOGI(TAG, "BSEC initialized with CONTINUOUS sampling rate (1Hz)");

    if (ret.bme69x_status != BME69X_OK) {
        printf("ERROR while initializing BME68x: %d\r\n", ret.bme69x_status);
        return;
    }
    if (ret.bsec_status < BSEC_OK) {
        printf("\nERROR while initializing BSEC library: %d\n", ret.bsec_status);
        return;
    } else if (ret.bsec_status > BSEC_OK) {
        printf("\nWARNING while initializing BSEC library: %d\n", ret.bsec_status);
    }

    ret.bsec_status = bsec_get_version(bsecInstance, &version);

    printf("BSEC Version : %u.%u.%u.%u\r\n", version.major, version.minor, version.major_bugfix, version.minor_bugfix);
    /*
    #if (OUTPUT_MODE == IAQ)
        static char *header = "Time(ms), IAQ,  IAQ_accuracy, Static_IAQ, Raw_Temperature(degC), Raw_Humidity(%%rH), Comp_Temperature(degC),
    Comp_Humidity(%%rH), Raw_pressure(Pa), Raw_Gas(ohms), Gas_percentage, CO2, bVOC, Stabilization_status, Run_in_status, Bsec_status\r\n";
    #else
        static char *header = "Time(ms), Class/Target_1_prediction, Class/Target_2_prediction, Class/Target_3_prediction,
    Class/Target_4_prediction, Prediction_accuracy_1, Prediction_accuracy_2, Prediction_accuracy_3, Prediction_accuracy_4, Raw_pressure(Pa),
    Raw_Temperature(degC),  Raw_Humidity(%%rH), Raw_Gas(ohm), Raw_Gas_Index(num), Bsec_status\r\n"; #endif

        printf(header);
    */
    bsec_iot_loop(state_save, get_timestamp_ms, output_ready);

    bme690_i2c_deinit();
}

static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t length = n_buffer;

    ESP_LOGI(TAG, "Loading BSEC state from NVS...");

    // Open NVS handle
    err = nvs_open(BME690_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS for reading: %s", esp_err_to_name(err));
        return 0;
    }

    // Get the state data
    err = nvs_get_blob(nvs_handle, BME690_NVS_STATE_KEY, state_buffer, &length);
    nvs_close(nvs_handle);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully loaded BSEC state from NVS (%lu bytes)", (unsigned long)length);
        return length;
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No saved BSEC state found in NVS");
    } else {
        ESP_LOGW(TAG, "Failed to load BSEC state: %s", esp_err_to_name(err));
    }

    return 0;
}

static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer) {
    memcpy(config_buffer, bsec_config_iaq, n_buffer);
    return n_buffer;
}

static uint32_t get_timestamp_ms() {
    uint32_t system_current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    return system_current_time;
}

// Variables to track the latest state for forced saves
static uint8_t *latest_state_buffer = NULL;
static uint32_t latest_state_length = 0;

// Forward declaration
static void bme690_force_state_save(void);

static void state_save(const uint8_t *state_buffer, uint32_t length) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    static uint32_t last_save_time = 0;
    uint32_t current_time = get_timestamp_ms();

    // Update the latest state buffer
    if (latest_state_buffer == NULL || latest_state_length < length) {
        latest_state_buffer = realloc(latest_state_buffer, length);
    }
    if (latest_state_buffer != NULL) {
        memcpy(latest_state_buffer, state_buffer, length);
        latest_state_length = length;
    }

    // Only save state periodically to reduce NVS wear
    if (current_time - last_save_time < BME690_STATE_SAVE_PERIOD_MS && last_save_time != 0) {
        return;
    }

    ESP_LOGI(TAG, "Saving BSEC state to NVS (%lu bytes)...", (unsigned long)length);

    // Open NVS handle
    err = nvs_open(BME690_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS for writing: %s", esp_err_to_name(err));
        return;
    }

    // Save the state data
    err = nvs_set_blob(nvs_handle, BME690_NVS_STATE_KEY, state_buffer, length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save BSEC state: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return;
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully saved BSEC state to NVS");
        last_save_time = current_time;
    } else {
        ESP_LOGE(TAG, "Failed to commit BSEC state: %s", esp_err_to_name(err));
    }
}

extern volatile bool flBMV080Published;

extern float extTempOffset;

static void output_ready(outputs_t *output) {
    static uint8_t last_iaq_accuracy = 0;
    uint32_t current_time = get_timestamp_ms();

    if (first_output) {
        startup_time = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        first_output = false;
        ESP_LOGI(TAG, "First BSEC output received - starting calibration period");
    }

    // Create data structure from BSEC output
    bme690_data_t sensor_data = {.temperature = output->compensated_temperature,
        .pressure = output->raw_pressure,
        .humidity = output->compensated_humidity,
        .iaq = output->iaq,
        .iaq_accuracy = output->iaq_accuracy,
        .co2_equivalent = output->co2_equivalent,
        .breath_voc_equivalent = output->breath_voc_equivalent,
        .static_iaq = output->static_iaq,
        .gas_percentage = output->gas_percentage,
        .stabilization_status = output->stabStatus,
        .run_in_status = output->runInStatus,
        .timestamp = current_time};

    // Add to buffer (replaces all the sb_add calls)
    bme690_buffer_add(&sensor_buffer, &sensor_data);

    // Log detailed status information
    uint32_t uptime = (xTaskGetTickCount() * portTICK_PERIOD_MS / 1000) - startup_time;

    // Log accuracy status with meaning
    const char *accuracy_desc = "Unknown";
    switch (output->iaq_accuracy) {
    case 0:
        accuracy_desc = "Stabilizing";
        break;
    case 1:
        accuracy_desc = "Low";
        break;
    case 2:
        accuracy_desc = "Medium";
        break;
    case 3:
        accuracy_desc = "High";
        break;
    }

    // Log stabilization and run-in status
    const char *stab_status = output->stabStatus ? "Stabilized" : "Stabilizing";
    const char *runin_status = output->runInStatus ? "Complete" : "Running";

    ESP_LOGI(TAG, "Uptime: %lu s, Accuracy: %d (%s), Stabilization: %s, Run-in: %s", (unsigned long)uptime, output->iaq_accuracy,
        accuracy_desc, stab_status, runin_status);

    // Force save when IAQ accuracy improves
    if (output->iaq_accuracy > last_iaq_accuracy) {
        ESP_LOGI(TAG, "IAQ accuracy improved: %d -> %d", last_iaq_accuracy, output->iaq_accuracy);
        last_iaq_accuracy = output->iaq_accuracy;
        // Force a save when accuracy improves
        if (latest_state_buffer != NULL && latest_state_length > 0) {
            bme690_force_state_save();
        }
    }

    // Existing conditional publishing logic - simplified
    bool should_publish = false;
    bool use_averaged = false;
    bme690_data_t data_to_publish;

    if (!PVLN_CFG_BSEC_OUTPUT_UPDATE_GATED_BY_BMV080) {
        // Immediate publishing with raw data (matches current behavior)
        data_to_publish = sensor_data;
        use_averaged = false;
        should_publish = true;
    } else if (PVLN_CFG_BSEC_OUTPUT_UPDATE_GATED_BY_BMV080 && flBMV080Published) {
        // Gated publishing with averaged data (matches current behavior)
        data_to_publish = bme690_buffer_get_averaged(&sensor_buffer);
        use_averaged = true;
        should_publish = true;
        flBMV080Published = false;
    }

    if (should_publish) {
        // Publish through data broker instead of direct JSON formatting
        sensor_broker_publish_bme690(&data_to_publish, use_averaged);

        // Log the published values (matches current logging)
        const char *prefix = use_averaged ? "Averaged - " : "";
        ESP_LOGI(TAG, "%sTemp: %.2f°C, Press: %.2f Pa, Hum: %.2f%%", prefix, data_to_publish.temperature, data_to_publish.pressure,
            data_to_publish.humidity);
        ESP_LOGI(TAG, "%sIAQ: %.2f (acc: %d), CO2: %.2f ppm, VOC: %.2f ppm", prefix, data_to_publish.iaq, data_to_publish.iaq_accuracy,
            data_to_publish.co2_equivalent, data_to_publish.breath_voc_equivalent);
        ESP_LOGI(TAG, "Raw gas: %.2f ohms, Gas percentage: %.2f%%, Static IAQ: %.2f", output->raw_gas, data_to_publish.gas_percentage,
            data_to_publish.static_iaq);
    }
    // led_set(LED_GREEN, LED_OFF);
    // gpio_deep_sleep_hold_en();

    /*
    #if (OUTPUT_MODE == IAQ)
        printf("%ld,%f,%u,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\r\n", (uint32_t)(output->timestamp/1000000),
                    output->iaq, output->iaq_accuracy, output->static_iaq, output->raw_temp, output->raw_humidity,
    output->compensated_temperature, output->compensated_humidity, output->raw_pressure, output->raw_gas, output->gas_percentage,
    output->co2_equivalent, output->breath_voc_equivalent, output->stabStatus, output->runInStatus); #else
        printf("%ld,%f,%f,%f,%f,%u,%u,%u,%u,%f,%f,%f,%f,%u\r\n", (uint32_t)(output->timestamp/1000000),
                    output->gas_estimate_1, output->gas_estimate_2, output->gas_estimate_3, output->gas_estimate_4,
                    output->gas_accuracy_1, output->gas_accuracy_2, output->gas_accuracy_3, output->gas_accuracy_4,
                    output->raw_pressure, output->raw_temp, output->raw_humidity, output->raw_gas, output->raw_gas_index);
    #endif
    */
}

// Force save the latest BME690 state (used during shutdown)
static void bme690_force_state_save(void) {
    if (latest_state_buffer != NULL && latest_state_length > 0) {
        nvs_handle_t nvs_handle;
        esp_err_t err;

        ESP_LOGI(TAG, "Force saving BSEC state to NVS (%lu bytes)...", (unsigned long)latest_state_length);

        err = nvs_open(BME690_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            err = nvs_set_blob(nvs_handle, BME690_NVS_STATE_KEY, latest_state_buffer, latest_state_length);
            if (err == ESP_OK) {
                err = nvs_commit(nvs_handle);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "Successfully forced BSEC state save");
                }
            }
            nvs_close(nvs_handle);
        }

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to force save BSEC state: %s", esp_err_to_name(err));
        }
    }
}

void bme690_app_start() {
    xTaskCreate(&bme690_task, "bme690_task", 60 * 1024, NULL, configMAX_PRIORITIES - 1, NULL);

    // Register shutdown handler to save state
    ESP_ERROR_CHECK(esp_register_shutdown_handler(bme690_force_state_save));
}

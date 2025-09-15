#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "driver/temperature_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "config.h"
#include "sensor_data_broker.h"

static const char *TAG = "mqtt";

// Secrets moved to external configuration

// Home Assistant discovery topics
const char *TEMPLATE_HA_DISCOVERY_BME690_TEMP = "homeassistant/sensor/polverine_%s/temperature/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_HUMID = "homeassistant/sensor/polverine_%s/humidity/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_PRESS = "homeassistant/sensor/polverine_%s/pressure/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_IAQ = "homeassistant/sensor/polverine_%s/iaq/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_CO2 = "homeassistant/sensor/polverine_%s/co2/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_VOC = "homeassistant/sensor/polverine_%s/voc/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_IAQ_ACC = "homeassistant/sensor/polverine_%s/iaq_accuracy/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_STATIC_IAQ = "homeassistant/sensor/polverine_%s/static_iaq/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_GAS_PERCENTAGE = "homeassistant/sensor/polverine_%s/gas_percentage/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_STABILIZATION = "homeassistant/binary_sensor/polverine_%s/stabilization_status/config";
const char *TEMPLATE_HA_DISCOVERY_BME690_RUN_IN = "homeassistant/binary_sensor/polverine_%s/run_in_status/config";

const char *TEMPLATE_HA_DISCOVERY_BMV080_PM10 = "homeassistant/sensor/polverine_%s/pm10/config";
const char *TEMPLATE_HA_DISCOVERY_BMV080_PM25 = "homeassistant/sensor/polverine_%s/pm25/config";
const char *TEMPLATE_HA_DISCOVERY_BMV080_PM1 = "homeassistant/sensor/polverine_%s/pm1/config";
const char *TEMPLATE_HA_DISCOVERY_BMV080_OBSTRUCTED = "homeassistant/binary_sensor/polverine_%s/obstructed/config";
const char *TEMPLATE_HA_DISCOVERY_BMV080_OUT_OF_RANGE = "homeassistant/binary_sensor/polverine_%s/out_of_range/config";

// ESP32 system metrics discovery topics
const char *TEMPLATE_HA_DISCOVERY_ESP32_RSSI = "homeassistant/sensor/polverine_%s/wifi_rssi/config";
const char *TEMPLATE_HA_DISCOVERY_ESP32_HEAP = "homeassistant/sensor/polverine_%s/free_heap/config";
const char *TEMPLATE_HA_DISCOVERY_ESP32_UPTIME = "homeassistant/sensor/polverine_%s/uptime/config";
const char *TEMPLATE_HA_DISCOVERY_ESP32_TEMP = "homeassistant/sensor/polverine_%s/cpu_temperature/config";

// Home Assistant state topics
const char *TEMPLATE_HA_STATE_BME690 = "polverine/%s/bme690/state";
const char *TEMPLATE_HA_STATE_BMV080 = "polverine/%s/bmv080/state";
const char *TEMPLATE_HA_STATE_SYSTEM = "polverine/%s/system/state";
const char *TEMPLATE_HA_AVAILABILITY = "polverine/%s/availability";

// Configuration loaded from NVS

static polverine_mqtt_config_t current_mqtt_config = {0};
static bool mqtt_config_loaded = false;

extern char shortId[7];
static char device_name[64];
static char availability_topic[128];
static char bme690_state_topic[128];
static char bmv080_state_topic[128];
static char system_state_topic[128];

void mqtt_default_init(const char *id) {
    snprintf(device_name, sizeof(device_name), "Polverine %s", id);
    snprintf(availability_topic, sizeof(availability_topic), TEMPLATE_HA_AVAILABILITY, id);
    snprintf(bme690_state_topic, sizeof(bme690_state_topic), TEMPLATE_HA_STATE_BME690, id);
    snprintf(bmv080_state_topic, sizeof(bmv080_state_topic), TEMPLATE_HA_STATE_BMV080, id);
    snprintf(system_state_topic, sizeof(system_state_topic), TEMPLATE_HA_STATE_SYSTEM, id);
}

bool isConnected = false;
esp_mqtt_client_handle_t client = 0;
#define MQTT_CONNECTION_TIMEOUT_MB 15000
TickType_t mqtt_connection_start_time = 0;

// Send Home Assistant discovery messages
static void send_ha_discovery() {
    char topic[256];
    char payload[1024];

    char device_json[256];
    int dev_len = snprintf(device_json, sizeof(device_json),
        "{\"identifiers\":\"%s\",\"name\":\"%s\",\"manufacturer\":\"BlackIoT\",\"model\":\"Polverine Sensor\"}", shortId, device_name);
    if (dev_len <= 0 || dev_len >= (int)sizeof(device_json)) {
        ESP_LOGE(TAG, "Device JSON truncated (%d)", dev_len);
        return;
    }

    // BME690 Temperature
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_TEMP, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_temperature\","
        "\"name\":\"Temperature\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"temperature\","
        "\"unit_of_measurement\":\"°C\","
        "\"value_template\":\"{{ value_json.temperature }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Humidity
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_HUMID, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_humidity\","
        "\"name\":\"Humidity\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"humidity\","
        "\"unit_of_measurement\":\"%%\","
        "\"value_template\":\"{{ value_json.humidity }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Pressure
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_PRESS, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_pressure\","
        "\"name\":\"Pressure\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"pressure\","
        "\"unit_of_measurement\":\"hPa\","
        "\"value_template\":\"{{ value_json.pressure | float / 100 }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 IAQ
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_IAQ, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_iaq\","
        "\"name\":\"Indoor Air Quality\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"unit_of_measurement\":\"IAQ\","
        "\"value_template\":\"{{ value_json.iaq }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 CO2
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_CO2, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_co2\","
        "\"name\":\"CO2\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"carbon_dioxide\","
        "\"unit_of_measurement\":\"ppm\","
        "\"value_template\":\"{{ value_json.co2 }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 VOC
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_VOC, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_voc\","
        "\"name\":\"VOC\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"volatile_organic_compounds_parts\","
        "\"unit_of_measurement\":\"ppm\","
        "\"value_template\":\"{{ value_json.voc }}\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BMV080 PM10
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BMV080_PM10, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_pm10\","
        "\"name\":\"PM10\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"pm10\","
        "\"unit_of_measurement\":\"µg/m³\","
        "\"value_template\":\"{{ value_json.pm10 }}\","
        "\"device\":%s}",
        shortId, bmv080_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BMV080 PM2.5
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BMV080_PM25, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_pm25\","
        "\"name\":\"PM2.5\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"pm25\","
        "\"unit_of_measurement\":\"µg/m³\","
        "\"value_template\":\"{{ value_json.pm25 }}\","
        "\"device\":%s}",
        shortId, bmv080_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BMV080 PM1
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BMV080_PM1, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_pm1\","
        "\"name\":\"PM1\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"pm1\","
        "\"unit_of_measurement\":\"µg/m³\","
        "\"value_template\":\"{{ value_json.pm1 }}\","
        "\"device\":%s}",
        shortId, bmv080_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 IAQ Accuracy
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_IAQ_ACC, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_iaq_accuracy\","
        "\"name\":\"IAQ Accuracy\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"value_template\":\"{{ value_json.iaq_accuracy }}\","
        "\"icon\":\"mdi:gauge\","
        "\"state_class\":\"measurement\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Static IAQ
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_STATIC_IAQ, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_static_iaq\","
        "\"name\":\"Static IAQ\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"value_template\":\"{{ value_json.static_iaq }}\","
        "\"icon\":\"mdi:air-filter\","
        "\"state_class\":\"measurement\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Gas Percentage
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_GAS_PERCENTAGE, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_gas_percentage\","
        "\"name\":\"Gas Percentage\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"unit_of_measurement\":\"%%\","
        "\"value_template\":\"{{ value_json.gas_percentage }}\","
        "\"icon\":\"mdi:percent\","
        "\"state_class\":\"measurement\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Stabilization Status
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_STABILIZATION, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_stabilization_status\","
        "\"name\":\"Gas Sensor Stabilized\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"payload_on\":\"true\","
        "\"payload_off\":\"false\","
        "\"value_template\":\"{{ value_json.stabilization_status }}\","
        "\"icon\":\"mdi:check-circle\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BME690 Run-in Status
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BME690_RUN_IN, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_run_in_status\","
        "\"name\":\"Gas Sensor Run-in Complete\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"payload_on\":\"true\","
        "\"payload_off\":\"false\","
        "\"value_template\":\"{{ value_json.run_in_status }}\","
        "\"icon\":\"mdi:timer-check\","
        "\"device\":%s}",
        shortId, bme690_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BMV080 Obstructed
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BMV080_OBSTRUCTED, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_obstructed\","
        "\"name\":\"Sensor Obstructed\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"payload_on\":\"true\","
        "\"payload_off\":\"false\","
        "\"value_template\":\"{{ value_json.obstructed }}\","
        "\"device_class\":\"problem\","
        "\"device\":%s}",
        shortId, bmv080_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // BMV080 Out of Range
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_BMV080_OUT_OF_RANGE, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_out_of_range\","
        "\"name\":\"Measurement Out of Range\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"payload_on\":\"true\","
        "\"payload_off\":\"false\","
        "\"value_template\":\"{{ value_json.out_of_range }}\","
        "\"device_class\":\"problem\","
        "\"device\":%s}",
        shortId, bmv080_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // ESP32 WiFi RSSI
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_ESP32_RSSI, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_wifi_rssi\","
        "\"name\":\"WiFi Signal\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"signal_strength\","
        "\"unit_of_measurement\":\"dBm\","
        "\"value_template\":\"{{ value_json.rssi }}\","
        "\"icon\":\"mdi:wifi\","
        "\"device\":%s}",
        shortId, system_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // ESP32 Free Heap
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_ESP32_HEAP, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_free_heap\","
        "\"name\":\"Free Memory\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"unit_of_measurement\":\"B\","
        "\"value_template\":\"{{ value_json.free_heap }}\","
        "\"icon\":\"mdi:memory\","
        "\"state_class\":\"measurement\","
        "\"device\":%s}",
        shortId, system_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // ESP32 Uptime
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_ESP32_UPTIME, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_uptime\","
        "\"name\":\"Uptime\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"unit_of_measurement\":\"s\","
        "\"value_template\":\"{{ value_json.uptime }}\","
        "\"icon\":\"mdi:timer-outline\","
        "\"state_class\":\"total_increasing\","
        "\"device\":%s}",
        shortId, system_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    // ESP32 CPU Temperature
    snprintf(topic, sizeof(topic), TEMPLATE_HA_DISCOVERY_ESP32_TEMP, shortId);
    snprintf(payload, sizeof(payload),
        "{\"unique_id\":\"%s_cpu_temperature\","
        "\"name\":\"CPU Temperature\","
        "\"state_topic\":\"%s\","
        "\"availability_topic\":\"%s\","
        "\"device_class\":\"temperature\","
        "\"unit_of_measurement\":\"°C\","
        "\"value_template\":\"{{ value_json.cpu_temp }}\","
        "\"icon\":\"mdi:thermometer\","
        "\"state_class\":\"measurement\","
        "\"device\":%s}",
        shortId, system_state_topic, availability_topic, device_json);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, true);

    ESP_LOGI(TAG, "Home Assistant discovery messages sent");
}

// BME690 data callback handler
static void mqtt_bme690_data_handler(const bme690_data_t *data, bool is_averaged) {
    if (!isConnected || data == NULL)
        return;

    char payload[320];
    int written = snprintf(payload, sizeof(payload),
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f,"
        "\"iaq\":%.2f,\"co2\":%.2f,\"voc\":%.2f,"
        "\"iaq_accuracy\":%u,\"static_iaq\":%.2f,\"gas_percentage\":%.2f,"
        "\"stabilization_status\":\"%s\",\"run_in_status\":\"%s\","
        "\"data_type\":\"%s\",\"timestamp\":%u}",
        data->temperature, data->humidity, data->pressure, data->iaq, data->co2_equivalent, data->breath_voc_equivalent,
        (unsigned)data->iaq_accuracy, data->static_iaq, data->gas_percentage, data->stabilization_status ? "true" : "false",
        data->run_in_status ? "true" : "false", is_averaged ? "averaged" : "raw", (unsigned)data->timestamp);

    if (written <= 0 || written >= (int)sizeof(payload)) {
        ESP_LOGE(TAG, "BME690 JSON payload truncated (size=%d)", written);
        return;
    }

    esp_mqtt_client_publish(client, bme690_state_topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "Published %s BME690 data", is_averaged ? "averaged" : "raw");
}

// BMV080 data callback handler
static void mqtt_bmv080_data_handler(const bmv080_data_t *data) {
    if (!isConnected || data == NULL)
        return;

    char payload[192];
    int written = snprintf(payload, sizeof(payload),
        "{\"pm10\":%.2f,\"pm25\":%.2f,\"pm1\":%.2f,"
        "\"obstructed\":\"%s\",\"out_of_range\":\"%s\","
        "\"runtime\":%.2f,\"timestamp\":%u}",
        data->pm10, data->pm25, data->pm1, data->is_obstructed ? "true" : "false", data->is_outside_range ? "true" : "false", data->runtime,
        (unsigned)data->timestamp);

    if (written <= 0 || written >= (int)sizeof(payload)) {
        ESP_LOGE(TAG, "BMV080 JSON payload truncated (size=%d)", written);
        return;
    }

    esp_mqtt_client_publish(client, bmv080_state_topic, payload, 0, 1, 0);

    ESP_LOGI(TAG, "Published BMV080 data");
}

// Forward declarations
static void system_metrics_task(void *pvParameter);
void mqtt_start_system_metrics_task(void);
static TaskHandle_t system_metrics_task_handle = NULL;

// Publish ESP32 system metrics
void system_metrics_publish(void) {
    if (!isConnected)
        return;

    int32_t rssi = -127;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        rssi = ap_info.rssi;
    }

    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;

    static temperature_sensor_handle_t temp_handle = NULL;
    if (temp_handle == NULL) {
        temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
        temperature_sensor_install(&temp_sensor_config, &temp_handle);
        temperature_sensor_enable(temp_handle);
    }
    float cpu_temp = -273.0f;
    if (temperature_sensor_get_celsius(temp_handle, &cpu_temp) != ESP_OK) {
        cpu_temp = -273.0f;
    }

    char payload[160];
    int len = snprintf(payload, sizeof(payload), "{\"rssi\":%ld,\"free_heap\":%lu,\"uptime\":%lu,\"cpu_temp\":%.2f}", (long)rssi,
        (unsigned long)free_heap, (unsigned long)uptime, cpu_temp);

    if (len <= 0 || len >= (int)sizeof(payload)) {
        ESP_LOGE(TAG, "System metrics JSON truncated (len=%d)", len);
        return;
    }

    esp_mqtt_client_publish(client, system_state_topic, payload, 0, 1, 0);
}

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        isConnected = true;
        mqtt_connection_start_time = 0;

        // Publish online status FIRST with QoS 1 and retain
        int msg_id = esp_mqtt_client_publish(client, availability_topic, "online", 0, 1, true);
        ESP_LOGI(TAG, "Published availability 'online' to %s, msg_id=%d", availability_topic, msg_id);

        // Wait a bit to ensure availability is published before discovery
        vTaskDelay(pdMS_TO_TICKS(100));

        // Send Home Assistant discovery messages
        send_ha_discovery();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        isConnected = false;

        // Attempt to reconnect with shorter delay for better availability
        ESP_LOGI(TAG, "Will attempt to reconnect in 2 seconds...");
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Get MQTT configuration for detailed logging
        if (mqtt_config_loaded) {
            ESP_LOGI(TAG, "Attempting to reconnect to MQTT broker...");
            ESP_LOGI(TAG, "  Broker URI: %s", current_mqtt_config.uri);
            ESP_LOGI(TAG, "  Username: %s", current_mqtt_config.username);
            ESP_LOGI(TAG, "  Password: %s", current_mqtt_config.password);
            ESP_LOGI(TAG, "  Client ID: %s", current_mqtt_config.client_id);
        } else {
            ESP_LOGE(TAG, "MQTT configuration incomplete, cannot log connection details");
        }

        esp_err_t err = esp_mqtt_client_reconnect(client);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initiate reconnection: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Reconnection initiated");
            mqtt_connection_start_time = xTaskGetTickCount();
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    case MQTT_EVENT_PUBLISHED:
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGD(TAG, "MQTT_EVENT_DATA");
        ESP_LOGD(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
        ESP_LOGD(TAG, "DATA=%.*s", event->data_len, event->data);
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void) {
    ESP_LOGI(TAG, "Starting Home Assistant MQTT application...");

    if (config_load_mqtt(&current_mqtt_config, shortId)) {
        mqtt_config_loaded = true;
        ESP_LOGI(TAG, "MQTT configuration loaded: URI=%s, ClientID=%s", current_mqtt_config.uri, current_mqtt_config.client_id);
    } else {
        ESP_LOGE(TAG, "Failed to load MQTT configuration");
        mqtt_config_loaded = false;
        return;
    }

    if (!mqtt_config_loaded) {
        ESP_LOGE(TAG, "No MQTT configuration available");
        return;
    }

    // Configure MQTT client directly from struct
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = current_mqtt_config.uri;
    mqtt_cfg.credentials.username = current_mqtt_config.username;
    mqtt_cfg.credentials.authentication.password = current_mqtt_config.password;
    mqtt_cfg.credentials.client_id = current_mqtt_config.client_id;

    // Set last will message for availability
    mqtt_cfg.session.last_will.topic = availability_topic;
    mqtt_cfg.session.last_will.msg = "offline";
    mqtt_cfg.session.last_will.qos = 1;
    mqtt_cfg.session.last_will.retain = true;

    mqtt_cfg.network.timeout_ms = 10000;
    mqtt_cfg.session.keepalive = 30; // Increased for better stability

    // Log detailed connection information
    ESP_LOGI(TAG, "MQTT client configuration prepared:");
    ESP_LOGI(TAG, "  Broker URI: %s", mqtt_cfg.broker.address.uri);
    ESP_LOGI(TAG, "  Username: %s", mqtt_cfg.credentials.username);
    ESP_LOGI(TAG, "  Password: %s", mqtt_cfg.credentials.authentication.password);
    ESP_LOGI(TAG, "  Client ID: %s", mqtt_cfg.credentials.client_id);
    ESP_LOGI(TAG, "  Keepalive: %d seconds", mqtt_cfg.session.keepalive);
    ESP_LOGI(TAG, "  Timeout: %d ms", mqtt_cfg.network.timeout_ms);

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client!");
        return;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    mqtt_connection_start_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Attempting initial connection to MQTT broker...");
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        mqtt_connection_start_time = 0;
        return;
    }

    ESP_LOGI(TAG, "MQTT client started successfully");

    // Register for sensor data callbacks
    sensor_broker_register_bme690_callback(mqtt_bme690_data_handler);
    sensor_broker_register_bmv080_callback(mqtt_bmv080_data_handler);
    ESP_LOGI(TAG, "Sensor data callbacks registered");

    // Start system metrics reporting task
    mqtt_start_system_metrics_task();
    ESP_LOGI(TAG, "System metrics task started");
}

// Task to periodically publish system metrics and availability
static void system_metrics_task(void *pvParameter) {
    const TickType_t xDelay = 30000 / portTICK_PERIOD_MS;             // Publish every 30 seconds
    const TickType_t xAvailabilityDelay = 60000 / portTICK_PERIOD_MS; // Availability every 60 seconds
    TickType_t lastAvailabilityTime = 0;

    for (;;) {
        vTaskDelay(xDelay);

        if (isConnected) {
            // Publish system metrics
            system_metrics_publish();

            // Check if it's time to send availability heartbeat
            TickType_t currentTime = xTaskGetTickCount();
            if ((currentTime - lastAvailabilityTime) >= xAvailabilityDelay) {
                int msg_id = esp_mqtt_client_publish(client, availability_topic, "online", 0, 1, true);
                ESP_LOGI(TAG, "Availability heartbeat sent, msg_id=%d", msg_id);
                lastAvailabilityTime = currentTime;
            }
        }
    }
}

void mqtt_start_system_metrics_task(void) {
    // Only create task if it doesn't already exist
    if (system_metrics_task_handle == NULL) {
        xTaskCreate(&system_metrics_task, "system_metrics_task", 4096, NULL, 5, &system_metrics_task_handle);
        ESP_LOGI(TAG, "System metrics task created");
    } else {
        ESP_LOGI(TAG, "System metrics task already exists");
    }
}

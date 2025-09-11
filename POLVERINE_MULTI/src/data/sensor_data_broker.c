#include "sensor_data_broker.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SENSOR_BROKER";

#define MAX_BME690_CALLBACKS 4
#define MAX_BMV080_CALLBACKS 4

// Callback arrays
static bme690_data_callback_t bme690_callbacks[MAX_BME690_CALLBACKS] = {0};
static bmv080_data_callback_t bmv080_callbacks[MAX_BMV080_CALLBACKS] = {0};

// Callback counters
static uint8_t bme690_callback_count = 0;
static uint8_t bmv080_callback_count = 0;

// Statistics for debugging
static uint32_t bme690_publish_count = 0;
static uint32_t bmv080_publish_count = 0;

void sensor_broker_init(void)
{
    // Clear all callbacks
    memset(bme690_callbacks, 0, sizeof(bme690_callbacks));
    memset(bmv080_callbacks, 0, sizeof(bmv080_callbacks));
    
    bme690_callback_count = 0;
    bmv080_callback_count = 0;
    
    bme690_publish_count = 0;
    bmv080_publish_count = 0;
    
    ESP_LOGI(TAG, "Sensor data broker initialized");
}

void sensor_broker_register_bme690_callback(bme690_data_callback_t callback)
{
    if (callback == NULL) {
        ESP_LOGW(TAG, "Attempted to register NULL BME690 callback");
        return;
    }
    
    if (bme690_callback_count >= MAX_BME690_CALLBACKS) {
        ESP_LOGE(TAG, "Maximum BME690 callbacks reached (%d)", MAX_BME690_CALLBACKS);
        return;
    }
    
    bme690_callbacks[bme690_callback_count] = callback;
    bme690_callback_count++;
    
    ESP_LOGI(TAG, "Registered BME690 callback #%d", bme690_callback_count);
}

void sensor_broker_register_bmv080_callback(bmv080_data_callback_t callback)
{
    if (callback == NULL) {
        ESP_LOGW(TAG, "Attempted to register NULL BMV080 callback");
        return;
    }
    
    if (bmv080_callback_count >= MAX_BMV080_CALLBACKS) {
        ESP_LOGE(TAG, "Maximum BMV080 callbacks reached (%d)", MAX_BMV080_CALLBACKS);
        return;
    }
    
    bmv080_callbacks[bmv080_callback_count] = callback;
    bmv080_callback_count++;
    
    ESP_LOGI(TAG, "Registered BMV080 callback #%d", bmv080_callback_count);
}

void sensor_broker_publish_bme690(const bme690_data_t *data, bool is_averaged)
{
    if (data == NULL) {
        ESP_LOGW(TAG, "Attempted to publish NULL BME690 data");
        return;
    }
    
    bme690_publish_count++;
    
    // Validate data ranges (basic sanity check)
    if (data->temperature < -40.0f || data->temperature > 85.0f) {
        ESP_LOGW(TAG, "BME690 temperature out of range: %.2f°C", data->temperature);
    }
    if (data->humidity < 0.0f || data->humidity > 100.0f) {
        ESP_LOGW(TAG, "BME690 humidity out of range: %.2f%%", data->humidity);
    }
    if (data->pressure < 30000.0f || data->pressure > 110000.0f) {
        ESP_LOGW(TAG, "BME690 pressure out of range: %.2f Pa", data->pressure);
    }
    
    // Call all registered callbacks
    for (uint8_t i = 0; i < bme690_callback_count; i++) {
        if (bme690_callbacks[i] != NULL) {
            bme690_callbacks[i](data, is_averaged);
        }
    }
    
    ESP_LOGD(TAG, "Published BME690 data (%s) to %d callbacks", 
             is_averaged ? "averaged" : "raw", bme690_callback_count);
}

void sensor_broker_publish_bmv080(const bmv080_data_t *data)
{
    if (data == NULL) {
        ESP_LOGW(TAG, "Attempted to publish NULL BMV080 data");
        return;
    }
    
    bmv080_publish_count++;
    
    // Validate data ranges (basic sanity check)
    if (data->pm10 < 0.0f || data->pm10 > 1000.0f) {
        ESP_LOGW(TAG, "BMV080 PM10 out of expected range: %.2f µg/m³", data->pm10);
    }
    if (data->pm25 < 0.0f || data->pm25 > 1000.0f) {
        ESP_LOGW(TAG, "BMV080 PM2.5 out of expected range: %.2f µg/m³", data->pm25);
    }
    if (data->pm1 < 0.0f || data->pm1 > 1000.0f) {
        ESP_LOGW(TAG, "BMV080 PM1 out of expected range: %.2f µg/m³", data->pm1);
    }
    
    // Call all registered callbacks
    for (uint8_t i = 0; i < bmv080_callback_count; i++) {
        if (bmv080_callbacks[i] != NULL) {
            bmv080_callbacks[i](data);
        }
    }
    
    ESP_LOGD(TAG, "Published BMV080 data to %d callbacks", bmv080_callback_count);
}
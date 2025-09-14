/**
 * @file webserver_sensor.c
 * @brief Sensor data web server handlers
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_data_broker.h"
#include "webserver.h"

static const char *TAG = "web_sensor";

// Latest sensor data
static bme690_data_t latest_bme690_data = {0};
static bmv080_data_t latest_bmv080_data = {0};
static bool bme690_data_available = false;
static bool bmv080_data_available = false;

// Embedded compressed HTML files
extern const uint8_t sensor_dashboard_html_gz_start[] asm("_binary_sensor_dashboard_html_gz_start");
extern const uint8_t sensor_dashboard_html_gz_end[] asm("_binary_sensor_dashboard_html_gz_end");

// Sensor data callbacks
static void bme690_data_callback(const bme690_data_t *data, bool is_averaged) {
    if (data != NULL) {
        memcpy(&latest_bme690_data, data, sizeof(bme690_data_t));
        bme690_data_available = true;
        ESP_LOGD(TAG, "Updated BME690 data: T=%.2f°C, H=%.1f%%, P=%.1fPa, IAQ=%.1f", data->temperature, data->humidity, data->pressure,
            data->iaq);
    }
}

static void bmv080_data_callback(const bmv080_data_t *data) {
    if (data != NULL) {
        memcpy(&latest_bmv080_data, data, sizeof(bmv080_data_t));
        bmv080_data_available = true;
        ESP_LOGD(TAG, "Updated BMV080 data: PM1=%.1f, PM2.5=%.1f, PM10=%.1f µg/m³", data->pm1, data->pm25, data->pm10);
    }
}

// HTTP handler for the main dashboard page
static esp_err_t dashboard_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving sensor dashboard (compressed)");
    const size_t len = sensor_dashboard_html_gz_end - sensor_dashboard_html_gz_start;
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_send(req, (const char *)sensor_dashboard_html_gz_start, len);
    return ESP_OK;
}

// HTTP handler for the JSON data endpoint
static esp_err_t data_get_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "Serving sensor data JSON");

    // Create JSON object
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
        return ESP_FAIL;
    }

    // Add timestamp
    cJSON *timestamp = cJSON_CreateNumber(esp_timer_get_time() / 1000); // Convert to milliseconds
    cJSON_AddItemToObject(json, "timestamp", timestamp);

    // Add BME690 data
    if (bme690_data_available) {
        cJSON *bme690 = cJSON_CreateObject();
        cJSON_AddNumberToObject(bme690, "temperature", latest_bme690_data.temperature);
        cJSON_AddNumberToObject(bme690, "humidity", latest_bme690_data.humidity);
        cJSON_AddNumberToObject(bme690, "pressure", latest_bme690_data.pressure);
        cJSON_AddNumberToObject(bme690, "iaq", latest_bme690_data.iaq);
        cJSON_AddNumberToObject(bme690, "iaq_accuracy", latest_bme690_data.iaq_accuracy);
        cJSON_AddNumberToObject(bme690, "co2_equivalent", latest_bme690_data.co2_equivalent);
        cJSON_AddNumberToObject(bme690, "breath_voc_equivalent", latest_bme690_data.breath_voc_equivalent);
        cJSON_AddNumberToObject(bme690, "static_iaq", latest_bme690_data.static_iaq);
        cJSON_AddNumberToObject(bme690, "gas_percentage", latest_bme690_data.gas_percentage);
        cJSON_AddBoolToObject(bme690, "stabilization_status", latest_bme690_data.stabilization_status);
        cJSON_AddBoolToObject(bme690, "run_in_status", latest_bme690_data.run_in_status);
        cJSON_AddNumberToObject(bme690, "data_timestamp", latest_bme690_data.timestamp);
        cJSON_AddItemToObject(json, "bme690", bme690);
    } else {
        cJSON_AddNullToObject(json, "bme690");
    }

    // Add BMV080 data
    if (bmv080_data_available) {
        cJSON *bmv080 = cJSON_CreateObject();
        cJSON_AddNumberToObject(bmv080, "pm1", latest_bmv080_data.pm1);
        cJSON_AddNumberToObject(bmv080, "pm25", latest_bmv080_data.pm25);
        cJSON_AddNumberToObject(bmv080, "pm10", latest_bmv080_data.pm10);
        cJSON_AddBoolToObject(bmv080, "is_obstructed", latest_bmv080_data.is_obstructed);
        cJSON_AddBoolToObject(bmv080, "is_outside_range", latest_bmv080_data.is_outside_range);
        cJSON_AddNumberToObject(bmv080, "runtime", latest_bmv080_data.runtime);
        cJSON_AddNumberToObject(bmv080, "data_timestamp", latest_bmv080_data.timestamp);
        cJSON_AddItemToObject(json, "bmv080", bmv080);
    } else {
        cJSON_AddNullToObject(json, "bmv080");
    }

    // Add data availability status
    cJSON *status = cJSON_CreateObject();
    cJSON_AddBoolToObject(status, "bme690_available", bme690_data_available);
    cJSON_AddBoolToObject(status, "bmv080_available", bmv080_data_available);
    cJSON_AddItemToObject(json, "status", status);

    // Convert to string
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to serialize JSON");
        return ESP_FAIL;
    }

    // Send response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));

    // Cleanup
    free(json_string);
    cJSON_Delete(json);

    return ret;
}

esp_err_t webserver_register_sensor_handlers(httpd_handle_t server) {
    ESP_LOGI(TAG, "Registering sensor data handlers");

    // Register with sensor data broker for callbacks
    sensor_broker_register_bme690_callback(bme690_data_callback);
    sensor_broker_register_bmv080_callback(bmv080_data_callback);

    // Register URI handlers
    httpd_uri_t dashboard_uri = {.uri = "/", .method = HTTP_GET, .handler = dashboard_get_handler, .user_ctx = NULL};
    esp_err_t ret = httpd_register_uri_handler(server, &dashboard_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register dashboard handler");
        return ret;
    }

    httpd_uri_t data_uri = {.uri = "/data", .method = HTTP_GET, .handler = data_get_handler, .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &data_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register data handler");
        return ret;
    }

    ESP_LOGI(TAG, "Sensor data handlers registered successfully");
    return ESP_OK;
}
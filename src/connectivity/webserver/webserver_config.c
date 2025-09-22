/**
 * @file webserver_config.c
 * @brief Configuration web server handlers
 */

#include <string.h>
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "webserver.h"

static const char *TAG = "web_config";

// External device ID from main.c
extern char shortId[7];

// Embedded compressed HTML files
extern const uint8_t config_html_gz_start[] asm("_binary_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary_config_html_gz_end");
extern const uint8_t success_html_gz_start[] asm("_binary_success_html_gz_start");
extern const uint8_t success_html_gz_end[] asm("_binary_success_html_gz_end");

static esp_err_t config_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving configuration page (compressed)");
    const size_t len = config_html_gz_end - config_html_gz_start;
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_send(req, (const char *)config_html_gz_start, len);
    return ESP_OK;
}

static esp_err_t save_post_handler(httpd_req_t *req) {
    char buf[512];
    int ret, remaining = req->content_len;

    if (remaining > sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive data");
        return ESP_FAIL;
    }

    buf[ret] = '\0';
    ESP_LOGI(TAG, "Received configuration: %s", buf);

    // Parse form data (URL encoded)
    polverine_wifi_config_t wifi_cfg = {0};
    polverine_mqtt_config_t mqtt_cfg = {0};

    char *token = strtok(buf, "&");
    while (token != NULL) {
        char *key = token;
        char *value = strchr(token, '=');
        if (value) {
            *value++ = '\0';

            // URL decode value (basic - just handle + for space)
            char *src = value, *dst = value;
            while (*src) {
                if (*src == '+') {
                    *dst++ = ' ';
                    src++;
                } else if (*src == '%' && src[1] && src[2]) {
                    // Handle %XX encoding
                    int hex;
                    sscanf(src + 1, "%2x", &hex);
                    *dst++ = (char)hex;
                    src += 3;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';

            // Store values
            if (strcmp(key, "ssid") == 0) {
                strncpy(wifi_cfg.ssid, value, sizeof(wifi_cfg.ssid) - 1);
            } else if (strcmp(key, "pass") == 0) {
                strncpy(wifi_cfg.password, value, sizeof(wifi_cfg.password) - 1);
            } else if (strcmp(key, "mqtt_uri") == 0) {
                strncpy(mqtt_cfg.uri, value, sizeof(mqtt_cfg.uri) - 1);
            } else if (strcmp(key, "mqtt_user") == 0) {
                strncpy(mqtt_cfg.username, value, sizeof(mqtt_cfg.username) - 1);
            } else if (strcmp(key, "mqtt_pass") == 0) {
                strncpy(mqtt_cfg.password, value, sizeof(mqtt_cfg.password) - 1);
            }
        }
        token = strtok(NULL, "&");
    }

    // Save configuration
    bool wifi_saved = config_save_wifi(&wifi_cfg);
    bool mqtt_saved = config_save_mqtt(&mqtt_cfg);

    if (wifi_saved && mqtt_saved) {
        ESP_LOGI(TAG, "Configuration saved successfully");
        const size_t len = success_html_gz_end - success_html_gz_start;
        httpd_resp_set_type(req, "text/html; charset=utf-8");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, (const char *)success_html_gz_start, len);

        // Schedule restart after sending response
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save configuration");
    }

    return ESP_OK;
}

static esp_err_t current_config_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving current configuration JSON");

    // Create JSON object
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
        return ESP_FAIL;
    }

    // Load current WiFi configuration
    polverine_wifi_config_t wifi_cfg = {0};
    bool wifi_loaded = config_load_wifi(&wifi_cfg);

    // Load current MQTT configuration
    polverine_mqtt_config_t mqtt_cfg = {0};
    bool mqtt_loaded = config_load_mqtt(&mqtt_cfg, shortId);

    // Add WiFi configuration
    cJSON *wifi_json = cJSON_CreateObject();
    if (wifi_loaded) {
        cJSON_AddStringToObject(wifi_json, "ssid", wifi_cfg.ssid);
        // Don't send password for security reasons
        cJSON_AddStringToObject(wifi_json, "password", "");
    } else {
        cJSON_AddStringToObject(wifi_json, "ssid", "");
        cJSON_AddStringToObject(wifi_json, "password", "");
    }
    cJSON_AddItemToObject(json, "wifi", wifi_json);

    // Add MQTT configuration
    cJSON *mqtt_json = cJSON_CreateObject();
    if (mqtt_loaded) {
        cJSON_AddStringToObject(mqtt_json, "uri", mqtt_cfg.uri);
        cJSON_AddStringToObject(mqtt_json, "username", mqtt_cfg.username);
        // Don't send password for security reasons
        cJSON_AddStringToObject(mqtt_json, "password", "");
    } else {
        cJSON_AddStringToObject(mqtt_json, "uri", "");
        cJSON_AddStringToObject(mqtt_json, "username", "");
        cJSON_AddStringToObject(mqtt_json, "password", "");
    }
    cJSON_AddItemToObject(json, "mqtt", mqtt_json);

    // Add status
    cJSON_AddBoolToObject(json, "wifi_configured", wifi_loaded && strlen(wifi_cfg.ssid) > 0);
    cJSON_AddBoolToObject(json, "mqtt_configured", mqtt_loaded && strlen(mqtt_cfg.uri) > 0);

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

esp_err_t webserver_register_config_handlers(httpd_handle_t server) {
    ESP_LOGI(TAG, "Registering configuration handlers");

    // Register URI handlers
    httpd_uri_t config_uri = {.uri = "/config", .method = HTTP_GET, .handler = config_get_handler, .user_ctx = NULL};
    esp_err_t ret = httpd_register_uri_handler(server, &config_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register config handler");
        return ret;
    }

    httpd_uri_t save_uri = {.uri = "/config/save", .method = HTTP_POST, .handler = save_post_handler, .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &save_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register save handler");
        return ret;
    }

    httpd_uri_t current_config_uri = {.uri = "/config/get", .method = HTTP_GET, .handler = current_config_get_handler, .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &current_config_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register current config handler");
        return ret;
    }

    ESP_LOGI(TAG, "Configuration handlers registered successfully");
    return ESP_OK;
}
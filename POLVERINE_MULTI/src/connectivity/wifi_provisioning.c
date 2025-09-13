/**
 * @file wifi_provisioning.c
 * @brief WiFi provisioning implementation
 */

#include "wifi_provisioning.h"
#include "config.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "wifi_prov";

// SoftAP configuration
#define PROV_AP_SSID_PREFIX     "Polverine-"
#define PROV_AP_PASS            "12345678"  // Minimum 8 characters
#define PROV_AP_MAX_CONN        4

// Web server
static httpd_handle_t server = NULL;
static bool provisioning_active = false;
static char ap_ssid[32];

// Embedded compressed HTML files
extern const uint8_t _binary___web_config_html_gz_start[];
extern const uint8_t _binary___web_config_html_gz_end[];
extern const uint8_t _binary___web_success_html_gz_start[];
extern const uint8_t _binary___web_success_html_gz_end[];

static esp_err_t config_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serving configuration page (compressed)");
    const size_t len = _binary___web_config_html_gz_end - _binary___web_config_html_gz_start;
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, (const char*)_binary___web_config_html_gz_start, len);
    return ESP_OK;
}

static esp_err_t save_post_handler(httpd_req_t *req)
{
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
        const size_t len = _binary___web_success_html_gz_end - _binary___web_success_html_gz_start;
        httpd_resp_set_type(req, "text/html; charset=utf-8");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, (const char*)_binary___web_success_html_gz_start, len);
        
        // Schedule restart after sending response
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save configuration");
    }
    
    return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    // Redirect to config page
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/config");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    
    // Increase limits to handle more connections and headers
    config.max_open_sockets = 7;  // Default is 7, max allowed
    config.max_resp_headers = 16; // Increase from default 8
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;
    
    ESP_LOGI(TAG, "Starting web server on port %d", config.server_port);
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root);
        
        httpd_uri_t config_page = {
            .uri       = "/config",
            .method    = HTTP_GET,
            .handler   = config_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &config_page);
        
        httpd_uri_t save = {
            .uri       = "/save",
            .method    = HTTP_POST,
            .handler   = save_post_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &save);
        
        ESP_LOGI(TAG, "Web server started");
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

static void stop_webserver(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
}

bool provisioning_is_needed(void)
{
    polverine_wifi_config_t wifi_cfg;
    return !config_load_wifi(&wifi_cfg) || strlen(wifi_cfg.ssid) == 0;
}

esp_err_t provisioning_start(const char *device_id)
{
    if (provisioning_active) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Starting provisioning mode");
    
    // Create AP SSID
    snprintf(ap_ssid, sizeof(ap_ssid), "%s%s", PROV_AP_SSID_PREFIX, device_id);
    
    // Configure WiFi as AP
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ap_ssid),
            .channel = 1,
            .password = PROV_AP_PASS,
            .max_connection = PROV_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    
    if (strlen(PROV_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    strcpy((char*)wifi_config.ap.ssid, ap_ssid);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi AP started. SSID: %s, Password: %s", ap_ssid, PROV_AP_PASS);
    ESP_LOGI(TAG, "Connect to the AP and navigate to http://192.168.4.1");
    
    // Start web server
    start_webserver();
    
    provisioning_active = true;
    return ESP_OK;
}

void provisioning_stop(void)
{
    if (!provisioning_active) {
        return;
    }
    
    ESP_LOGI(TAG, "Stopping provisioning mode");
    
    stop_webserver();
    esp_wifi_stop();
    
    provisioning_active = false;
}

bool provisioning_is_active(void)
{
    return provisioning_active;
}

void provisioning_reset(void)
{
    ESP_LOGI(TAG, "Resetting provisioning - clearing saved credentials");
    config_clear_all();
}
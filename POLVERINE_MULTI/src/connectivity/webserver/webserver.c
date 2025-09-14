/**
 * @file webserver.c
 * @brief Unified web server implementation - common setup code
 */

#include "webserver.h"

#include <string.h>
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "WEBSERVER";

// Web server handle
static httpd_handle_t server = NULL;
static bool server_running = false;

// Forward declarations for handler registration functions
extern esp_err_t webserver_register_sensor_handlers(httpd_handle_t server);
extern esp_err_t webserver_register_config_handlers(httpd_handle_t server);

esp_err_t webserver_start(void) {
    if (server_running) {
        ESP_LOGW(TAG, "Web server is already running");
        return ESP_OK;
    }

    // Configure HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Increase limits to handle more connections and headers
    config.max_open_sockets = 7;  // Default is 7, max allowed
    config.max_resp_headers = 16; // Increase from default 8
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;
    config.server_port = 80; // Use port 80

    ESP_LOGI(TAG, "Starting unified web server on port %d", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        // Register sensor data handlers
        esp_err_t ret = webserver_register_sensor_handlers(server);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register sensor handlers");
            httpd_stop(server);
            return ESP_FAIL;
        }

        // Register configuration handlers
        ret = webserver_register_config_handlers(server);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register configuration handlers");
            httpd_stop(server);
            return ESP_FAIL;
        }

        server_running = true;
        ESP_LOGI(TAG, "Unified web server started successfully");
        ESP_LOGI(TAG, "Available endpoints:");
        ESP_LOGI(TAG, "  GET  / - Sensor dashboard");
        ESP_LOGI(TAG, "  GET  /data - Sensor data (JSON)");
        ESP_LOGI(TAG, "  GET  /config - Configuration page");
        ESP_LOGI(TAG, "  GET  /config/get - Current config (JSON)");
        ESP_LOGI(TAG, "  POST /config/save - Save configuration");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
        return ESP_FAIL;
    }
}

esp_err_t webserver_stop(void) {
    if (!server_running) {
        ESP_LOGW(TAG, "Web server is not running");
        return ESP_OK;
    }

    if (server) {
        httpd_stop(server);
        server = NULL;
        server_running = false;
        ESP_LOGI(TAG, "Web server stopped");
        return ESP_OK;
    }

    return ESP_FAIL;
}

bool webserver_is_running(void) {
    return server_running;
}
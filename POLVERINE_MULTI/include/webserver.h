/**
 * @file webserver.h
 * @brief Unified web server for sensor data and configuration
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stdbool.h>
#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start the unified web server
 *
 * This web server serves:
 * - GET / : Main sensor dashboard page
 * - GET /data : JSON endpoint with current sensor data
 * - GET /config : Configuration page for WiFi and MQTT settings
 * - GET /config/get : Get current configuration (JSON)
 * - POST /config/save : Save configuration endpoint
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t webserver_start(void);

/**
 * @brief Stop the web server
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t webserver_stop(void);

/**
 * @brief Check if the web server is running
 *
 * @return true if running, false otherwise
 */
bool webserver_is_running(void);

/**
 * @brief Register sensor data handlers with the web server
 *
 * @param server HTTP server handle
 * @return ESP_OK on success
 */
esp_err_t webserver_register_sensor_handlers(httpd_handle_t server);

/**
 * @brief Register configuration handlers with the web server
 *
 * @param server HTTP server handle
 * @return ESP_OK on success
 */
esp_err_t webserver_register_config_handlers(httpd_handle_t server);

#ifdef __cplusplus
}
#endif

#endif // WEBSERVER_H
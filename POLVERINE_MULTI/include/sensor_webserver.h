#ifndef SENSOR_WEBSERVER_H
#define SENSOR_WEBSERVER_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start the sensor data web server
 *
 * This web server serves:
 * - GET / : Main page with sensor data display
 * - GET /data : JSON endpoint with current sensor data
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t sensor_webserver_start(void);

/**
 * @brief Stop the sensor data web server
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t sensor_webserver_stop(void);

/**
 * @brief Check if the sensor web server is running
 *
 * @return true if running, false otherwise
 */
bool sensor_webserver_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_WEBSERVER_H
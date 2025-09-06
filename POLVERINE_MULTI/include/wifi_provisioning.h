/**
 * @file wifi_provisioning.h
 * @brief WiFi provisioning with SoftAP and web portal
 */

#ifndef WIFI_PROVISIONING_H_
#define WIFI_PROVISIONING_H_

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if device needs provisioning
 * @return true if provisioning is needed, false otherwise
 */
bool provisioning_is_needed(void);

/**
 * Start provisioning mode
 * Creates a SoftAP and starts web server for configuration
 * @param device_id Short device ID for AP name
 * @return ESP_OK on success
 */
esp_err_t provisioning_start(const char *device_id);

/**
 * Stop provisioning mode
 */
void provisioning_stop(void);

/**
 * Check if provisioning is currently active
 * @return true if provisioning is active
 */
bool provisioning_is_active(void);

/**
 * Reset provisioning (clear saved credentials)
 */
void provisioning_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_PROVISIONING_H_ */
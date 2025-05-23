/**
 * @file config.h
 * @brief Configuration management for Polverine system
 * 
 * This file handles loading configuration from NVS (Non-Volatile Storage)
 * with fallback to environment-based defaults.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configuration structure for WiFi
typedef struct {
    char ssid[32];
    char password[64];
} polverine_wifi_config_t;

// Configuration structure for MQTT
typedef struct {
    char uri[128];
    char username[64];
    char password[64];
    char client_id[32];
} polverine_mqtt_config_t;

/**
 * Initialize configuration system
 * @return true if successful, false otherwise
 */
bool config_init(void);

/**
 * Load WiFi configuration from NVS
 * @param config Pointer to wifi_config_t structure to fill
 * @return true if configuration loaded successfully, false otherwise
 */
bool config_load_wifi(polverine_wifi_config_t *config);

/**
 * Save WiFi configuration to NVS
 * @param config Pointer to polverine_wifi_config_t structure to save
 * @return true if configuration saved successfully, false otherwise
 */
bool config_save_wifi(const polverine_wifi_config_t *config);

/**
 * Load MQTT configuration from NVS
 * @param config Pointer to polverine_mqtt_config_t structure to fill
 * @param device_id Device ID to use for client_id if not configured
 * @return true if configuration loaded successfully, false otherwise
 */
bool config_load_mqtt(polverine_mqtt_config_t *config, const char *device_id);

/**
 * Save MQTT configuration to NVS
 * @param config Pointer to polverine_mqtt_config_t structure to save
 * @return true if configuration saved successfully, false otherwise
 */
bool config_save_mqtt(const polverine_mqtt_config_t *config);

/**
 * Clear all configuration from NVS
 * @return true if successful, false otherwise
 */
bool config_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H_ */
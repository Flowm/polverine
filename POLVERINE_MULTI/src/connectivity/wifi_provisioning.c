/**
 * @file wifi_provisioning.c
 * @brief WiFi provisioning implementation
 */

#include "wifi_provisioning.h"

#include <string.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "cJSON.h"

#include "config.h"

static const char *TAG = "wifi_prov";

// SoftAP configuration
#define PROV_AP_SSID_PREFIX "Polverine-"
#define PROV_AP_PASS        "12345678" // Minimum 8 characters
#define PROV_AP_MAX_CONN    4

static bool provisioning_active = false;
static char ap_ssid[32];

bool provisioning_is_needed(void) {
    polverine_wifi_config_t wifi_cfg;
    return !config_load_wifi(&wifi_cfg) || strlen(wifi_cfg.ssid) == 0;
}

esp_err_t provisioning_start(const char *device_id) {
    if (provisioning_active) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting provisioning mode");

    // Create AP SSID
    snprintf(ap_ssid, sizeof(ap_ssid), "%s%s", PROV_AP_SSID_PREFIX, device_id);

    // Configure WiFi as AP
    wifi_config_t wifi_config = {
        .ap =
            {
                .ssid_len = strlen(ap_ssid),
                .channel = 1,
                .password = PROV_AP_PASS,
                .max_connection = PROV_AP_MAX_CONN,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .pmf_cfg =
                    {
                        .required = false,
                    },
            },
    };

    if (strlen(PROV_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    strcpy((char *)wifi_config.ap.ssid, ap_ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID: %s, Password: %s", ap_ssid, PROV_AP_PASS);
    ESP_LOGI(TAG, "Connect to the AP and navigate to http://192.168.4.1");

    provisioning_active = true;
    return ESP_OK;
}

void provisioning_stop(void) {
    if (!provisioning_active) {
        return;
    }

    ESP_LOGI(TAG, "Stopping provisioning mode");

    esp_wifi_stop();

    provisioning_active = false;
}

bool provisioning_is_active(void) {
    return provisioning_active;
}

void provisioning_reset(void) {
    ESP_LOGI(TAG, "Resetting provisioning - clearing saved credentials");
    config_clear_all();
}
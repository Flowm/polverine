/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "common_private.h"
#include "protocol_common.h"

/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created withing common polverine_connect component are prefixed with the module TAG,
 * so it returns true if the specified netif is owned by this module
 */
bool is_our_netif(const char *prefix, esp_netif_t *netif) {
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}

esp_err_t polverine_connect(void) {
    static const char *TAG = "connect";
    ESP_LOGI(TAG, "Starting polverine_connect...");

    ESP_LOGI(TAG, "Calling wifi_connect()...");
    esp_err_t ret = wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "wifi_connect failed with error: 0x%x", ret);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "wifi_connect successful");

    ESP_LOGI(TAG, "Registering shutdown handler...");
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_shutdown));
    ESP_LOGI(TAG, "Shutdown handler registered");

    ESP_LOGI(TAG, "polverine_connect completed successfully");
    return ESP_OK;
}

esp_err_t polverine_disconnect(void) {
    wifi_shutdown();
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&wifi_shutdown));
    return ESP_OK;
}

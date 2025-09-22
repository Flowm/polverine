/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "button_handler.h"
#include "config.h"
#include "led_control.h"
#include "protocol_common.h"
#include "sensor_data_broker.h"
#include "system_init.h"
#include "webserver.h"
#include "wifi_provisioning.h"

extern void bmv080_app_start();
extern void bme690_app_start();
extern void mqtt_app_start(void);
static const char *TAG = "main";
char uniqueId[13] = {0};
char shortId[7] = {0};

extern void mqtt_default_init(const char *id);

void app_main(void) {

    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    snprintf(uniqueId, sizeof(uniqueId), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(shortId, sizeof(shortId), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    mqtt_default_init(shortId);

    led_init();
    pm_init();

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Enable more verbose logging for troubleshooting
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("wifi_connect", ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "Initializing NVS flash...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "NVS flash initialized");

    // Initialize configuration system
    if (!config_init()) {
        ESP_LOGE(TAG, "Failed to initialize configuration system");
    }

    // Initialize button handler for configuration reset
    button_handler_init();

    ESP_LOGI(TAG, "Initializing network interface...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_LOGI(TAG, "Network interface initialized");

    ESP_LOGI(TAG, "Creating default event loop...");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "Default event loop created");

    // Initialize WiFi based on provisioning status
    if (provisioning_is_needed()) {
        ESP_LOGI(TAG, "No WiFi credentials found. Creating WiFi AP...");

        // Create WiFi AP
        esp_netif_create_default_wifi_ap();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // Start provisioning (this will configure the AP)
        provisioning_start(shortId);
    } else {
        ESP_LOGI(TAG, "WiFi credentials found. Connecting to WiFi...");

        // Connect to configured WiFi
        esp_err_t ret = polverine_connect();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "WiFi connection failed with error: 0x%x", ret);
            ESP_LOGI(TAG, "Falling back to provisioning mode...");

            // Failed to connect, create AP and start provisioning
            esp_netif_create_default_wifi_ap();
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));

            provisioning_start(shortId);
        } else {
            ESP_LOGI(TAG, "WiFi connection established");
        }
    }

    // Initialize sensor data broker
    ESP_LOGI(TAG, "Initializing sensor data broker...");
    sensor_broker_init();
    ESP_LOGI(TAG, "Sensor data broker initialized");

    ESP_LOGI(TAG, "Starting BMV080 application...");
    bmv080_app_start();
    ESP_LOGI(TAG, "BMV080 application started");

    ESP_LOGI(TAG, "Starting BME690 application...");
    bme690_app_start();
    ESP_LOGI(TAG, "BME690 application started");

    ESP_LOGI(TAG, "Starting MQTT application...");
    mqtt_app_start();
    ESP_LOGI(TAG, "MQTT application started");

    // Start unified web server
    ESP_LOGI(TAG, "Starting unified web server...");
    if (webserver_start() == ESP_OK) {
        ESP_LOGI(TAG, "Unified web server started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start unified web server");
    }

    // Example LED usage:
    // led_flash(LED_GREEN);                    // Quick success indication
    // led_set(LED_RED, LED_ON);                // Turn on red LED
    // led_set_rgb(LED_ON, LED_OFF, LED_ON);    // Custom RGB combination
    // led_all_off();                           // Turn off all LEDs

    ESP_LOGI(TAG, "Startup complete!");
}

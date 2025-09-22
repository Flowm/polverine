/**
 * @file button_handler.c
 * @brief Button handler implementation
 */

#include "button_handler.h"

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"

static const char *TAG = "button";

#define BOOT_BUTTON_GPIO   0    // GPIO0 is typically the BOOT button
#define LONG_PRESS_TIME_MS 5000 // 5 seconds for config reset

static void button_task(void *pvParameters) {
    uint32_t press_start = 0;
    bool button_pressed = false;

    while (1) {
        int level = gpio_get_level(BOOT_BUTTON_GPIO);

        if (level == 0) { // Button pressed (active low)
            if (!button_pressed) {
                button_pressed = true;
                press_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
                ESP_LOGI(TAG, "Button pressed");
            } else {
                // Check if long press
                uint32_t press_duration = (xTaskGetTickCount() * portTICK_PERIOD_MS) - press_start;
                if (press_duration >= LONG_PRESS_TIME_MS) {
                    ESP_LOGW(TAG, "Long press detected - resetting configuration");

                    // Clear all configuration
                    config_clear_all();

                    // Restart to enter provisioning mode
                    ESP_LOGI(TAG, "Restarting device...");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    esp_restart();
                }
            }
        } else { // Button released
            if (button_pressed) {
                uint32_t press_duration = (xTaskGetTickCount() * portTICK_PERIOD_MS) - press_start;
                ESP_LOGI(TAG, "Button released after %lu ms", (unsigned long)press_duration);
                button_pressed = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
    }
}

void button_handler_init(void) {
    // Configure BOOT button as input
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BOOT_BUTTON_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 1, // Enable pull-up
    };
    gpio_config(&io_conf);

    // Create button monitoring task
    xTaskCreate(button_task, "button", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "Button handler initialized. Hold BOOT button for %d seconds to reset configuration.", LONG_PRESS_TIME_MS / 1000);
}
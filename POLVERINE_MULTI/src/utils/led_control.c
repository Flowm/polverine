/**
 * @file led_control.c
 * @brief LED control implementation for POLVERINE_MULTI project
 *
 * This module implements LED control functionality for the POLVERINE board's
 * RGB LEDs. It provides initialization, basic control, and status indication.
 */

#include "led_control.h"

#include "esp_rom_gpio.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Initialize all LED GPIOs
 */
void led_init(void) {
    // Initialize Red LED
    esp_rom_gpio_pad_select_gpio(R_LED_PIN);
    gpio_set_direction(R_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(R_LED_PIN, 0);
    gpio_hold_en(R_LED_PIN);

    // Initialize Green LED
    esp_rom_gpio_pad_select_gpio(G_LED_PIN);
    gpio_set_direction(G_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(G_LED_PIN, 0);
    gpio_hold_en(G_LED_PIN);

    // Initialize Blue LED
    esp_rom_gpio_pad_select_gpio(B_LED_PIN);
    gpio_set_direction(B_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(B_LED_PIN, 0);
    gpio_hold_en(B_LED_PIN);
}

/**
 * @brief Set individual LED state
 */
void led_set(led_color_t color, led_state_t state) {
    gpio_num_t pin;

    switch (color) {
    case LED_RED:
        pin = R_LED_PIN;
        break;
    case LED_GREEN:
        pin = G_LED_PIN;
        break;
    case LED_BLUE:
        pin = B_LED_PIN;
        break;
    default:
        return;
    }

    gpio_set_level(pin, state);
    gpio_hold_en(pin);
}

/**
 * @brief Turn off all LEDs
 */
void led_all_off(void) {
    led_set(LED_RED, LED_OFF);
    led_set(LED_GREEN, LED_OFF);
    led_set(LED_BLUE, LED_OFF);
}

/**
 * @brief Turn on all LEDs
 */
void led_all_on(void) {
    led_set(LED_RED, LED_ON);
    led_set(LED_GREEN, LED_ON);
    led_set(LED_BLUE, LED_ON);
}

/**
 * @brief Set RGB LED color
 */
void led_set_rgb(led_state_t red, led_state_t green, led_state_t blue) {
    led_set(LED_RED, red);
    led_set(LED_GREEN, green);
    led_set(LED_BLUE, blue);
}

/**
 * @brief Flash an LED for a brief period
 */
void led_flash(led_color_t color) {
    led_set(color, LED_ON);
    vTaskDelay(pdMS_TO_TICKS(1));
    led_set(color, LED_OFF);
}
/**
 * @file led_control.h
 * @brief LED control interface for POLVERINE_MULTI project
 *
 * This module provides an interface for controlling the RGB LEDs on the
 * POLVERINE board. It includes initialization, basic control functions,
 * and status indication patterns.
 */

#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "hal/gpio_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// LED pin definitions
#define R_LED_PIN 47 // GPIO pin connected to red LED
#define G_LED_PIN 48 // GPIO pin connected to green LED
#define B_LED_PIN 38 // GPIO pin connected to blue LED

// LED states
typedef enum {
    LED_OFF = 0,
    LED_ON = 1
} led_state_t;

// LED colors for convenience
typedef enum {
    LED_RED = 0,
    LED_GREEN = 1,
    LED_BLUE = 2
} led_color_t;

/**
 * @brief Initialize all LED GPIOs
 *
 * Configures the RGB LED pins as outputs and sets them to OFF state.
 * Should be called once during system initialization.
 */
void led_init(void);

/**
 * @brief Set individual LED state
 *
 * @param color LED color to control
 * @param state LED state (ON/OFF)
 */
void led_set(led_color_t color, led_state_t state);

/**
 * @brief Turn off all LEDs
 */
void led_all_off(void);

/**
 * @brief Turn on all LEDs
 */
void led_all_on(void);

/**
 * @brief Set RGB LED color
 *
 * @param red Red LED state
 * @param green Green LED state
 * @param blue Blue LED state
 */
void led_set_rgb(led_state_t red, led_state_t green, led_state_t blue);

/**
 * @brief Flash an LED for a brief period
 *
 * Turns on the specified LED, waits briefly, then turns it off.
 * Useful for quick status indications.
 *
 * @param color LED color to flash
 */
void led_flash(led_color_t color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LED_CONTROL_H_ */
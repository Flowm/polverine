/**
 *
 * @file peripherals.h
 *
 * @brief peripherals configuration
 *
 *
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus  
extern "C" {
#endif /* __cplusplus */

#define R_LED_PIN 47 // GPIO pin connected to onboard LED
#define G_LED_PIN 48 // GPIO pin connected to onboard LED
#define B_LED_PIN 38 // GPIO pin connected to onboard LED
#define BOOT_BUTTON GPIO_NUM_0

#ifdef __cplusplus  
}
#endif /* __cplusplus */

#endif /* PERIPHERALS_H_ */

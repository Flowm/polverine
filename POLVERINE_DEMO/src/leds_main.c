#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "peripherals.h"
#include "leds.h"

char stsMain = STATUS_BUSY;
char stsBMV080 = STATUS_BUSY;
char stsBME690 = STATUS_BUSY;

void delay(uint32_t period)
{
    vTaskDelay(period / portTICK_PERIOD_MS);
}


void leds_task(void *pvParameter)
{
  printf("Starting LEDs task\r\n");

  for(;;) {
    if((stsBME690 == STATUS_ERROR) && (stsBMV080 == STATUS_ERROR))
        gpio_set_level(R_LED_PIN, 1);
    else
        gpio_set_level(R_LED_PIN, 0);
    if(stsBME690 == STATUS_OK)
        gpio_set_level(G_LED_PIN, 1);
    else
        gpio_set_level(G_LED_PIN, 0);

    if(stsBMV080 == STATUS_OK)
        gpio_set_level(B_LED_PIN, 1);
    else
        gpio_set_level(B_LED_PIN, 0);

    if((stsBME690 == STATUS_OK) && (stsBMV080 == STATUS_OK))
    {
        delay(1);
    }
    else
    {
        delay(500);
    }
    // Check if the button is pressed
    int button_state = gpio_get_level(BOOT_BUTTON);
    if (button_state == 0) // Button pressed
    {
        gpio_set_level(B_LED_PIN, 1);
    }
    else // Button released
    {
        gpio_set_level(B_LED_PIN, 0);
    }
    gpio_set_level(R_LED_PIN, 0);
    gpio_set_level(G_LED_PIN, 0);
    if((stsBME690 == STATUS_OK) && (stsBMV080 == STATUS_OK))
    {
        delay(999);
    }
    else
    {
        delay(500);
    }
    }
}


void leds_app_start() 
{
  xTaskCreate(&leds_task, "leds_task", 60 * 1024, NULL, configMAX_PRIORITIES - 1, NULL);
}

#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "driver/i2c_master.h"

#include "bme69x.h"

/* Exported functions prototypes ---------------------------------------------*/
esp_err_t bme690_i2c_init(void);
esp_err_t bme690_i2c_deinit(void);

/* I2C read and write functions */
BME69X_INTF_RET_TYPE bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
BME69X_INTF_RET_TYPE bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

/* Delay function */
void bme69x_delay_us(uint32_t period, void *intf_ptr);

/* Interface initialization */
void bme69x_interface_init(struct bme69x_dev *bme, uint8_t intf, uint8_t sen_no);

#ifdef __cplusplus
}
#endif

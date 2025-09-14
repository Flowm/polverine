#include "bme690_io.h"

#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bme69x.h"

/* I2C GPIO Configuration */
#define I2C_MASTER_SCL_IO  21        // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO  14        // GPIO number for I2C master data
#define I2C_MASTER_NUM     I2C_NUM_0 // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 100000    // I2C master clock frequency
#define BME690_SENSOR_ADDR 0x76      // BME690 sensor address

static i2c_master_dev_handle_t dev_handle;
static i2c_master_bus_handle_t bus_handle;

esp_err_t bme690_i2c_init(void) {
    static i2c_master_bus_config_t i2c_mst_config_1 = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config_1, &bus_handle));

    static i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME690_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    return ESP_OK;
}

esp_err_t bme690_i2c_deinit(void) {
    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    return ESP_OK;
}

BME69X_INTF_RET_TYPE bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t device_addr = *(uint8_t *)intf_ptr;

    (void)intf_ptr;

    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, reg_data, len, -1);
}

BME69X_INTF_RET_TYPE bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    static uint8_t buffer[257];

    (void)intf_ptr;

    if (len < 256) {
        buffer[0] = reg_addr;
        memcpy(&buffer[1], reg_data, len);
        return i2c_master_transmit(dev_handle, buffer, len + 1, -1);
    } else
        return -1;
}

void bme69x_delay_us(uint32_t period, void *intf_ptr) {
    (void)intf_ptr;
    vTaskDelay(period / (portTICK_PERIOD_MS * 1000));
}

void bme69x_interface_init(struct bme69x_dev *bme, uint8_t intf, uint8_t sen_no) {
    if (bme == NULL)
        return;

    /* Bus configuration : I2C */
    bme->read = bme69x_i2c_read;
    bme->write = bme69x_i2c_write;
    bme->intf = BME69X_I2C_INTF;
    bme->delay_us = bme69x_delay_us;
    bme->intf_ptr = 0;
    bme->amb_temp = 22; /* The ambient temperature in deg C is used for defining the heater temperature */
}

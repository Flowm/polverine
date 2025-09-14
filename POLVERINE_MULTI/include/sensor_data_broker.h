#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BME690 sensor data structure
 */
typedef struct {
    float temperature;           // Compensated temperature in °C
    float pressure;              // Raw pressure in Pa
    float humidity;              // Compensated humidity in %RH
    float iaq;                   // Indoor Air Quality index
    uint8_t iaq_accuracy;        // IAQ accuracy (0-3)
    float co2_equivalent;        // CO2 equivalent in ppm
    float breath_voc_equivalent; // Breath VOC equivalent in ppm
    float static_iaq;            // Static IAQ value
    float gas_percentage;        // Gas sensor percentage
    bool stabilization_status;   // Gas sensor stabilization status
    bool run_in_status;          // Gas sensor run-in status
    uint32_t timestamp;          // Timestamp in milliseconds
} bme690_data_t;

/**
 * @brief BMV080 sensor data structure
 */
typedef struct {
    float pm10;            // PM10 mass concentration in µg/m³
    float pm25;            // PM2.5 mass concentration in µg/m³
    float pm1;             // PM1 mass concentration in µg/m³
    bool is_obstructed;    // Sensor obstruction status
    bool is_outside_range; // Measurement out of range status
    float runtime;         // Runtime in seconds
    uint32_t timestamp;    // Timestamp in milliseconds
} bmv080_data_t;

/**
 * @brief Callback function types for sensor data
 */
typedef void (*bme690_data_callback_t)(const bme690_data_t *data, bool is_averaged);
typedef void (*bmv080_data_callback_t)(const bmv080_data_t *data);

/**
 * @brief Initialize the sensor data broker
 */
void sensor_broker_init(void);

/**
 * @brief Register callback for BME690 sensor data
 * @param callback Function to call when BME690 data is available
 */
void sensor_broker_register_bme690_callback(bme690_data_callback_t callback);

/**
 * @brief Register callback for BMV080 sensor data
 * @param callback Function to call when BMV080 data is available
 */
void sensor_broker_register_bmv080_callback(bmv080_data_callback_t callback);

/**
 * @brief Publish BME690 sensor data to all registered callbacks
 * @param data Pointer to BME690 sensor data
 * @param is_averaged Whether the data contains averaged values
 */
void sensor_broker_publish_bme690(const bme690_data_t *data, bool is_averaged);

/**
 * @brief Publish BMV080 sensor data to all registered callbacks
 * @param data Pointer to BMV080 sensor data
 */
void sensor_broker_publish_bmv080(const bmv080_data_t *data);

#ifdef __cplusplus
}
#endif

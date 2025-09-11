#ifndef SENSOR_BUFFER_H
#define SENSOR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_data_broker.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SENSOR_SAMPLES 60

/**
 * @brief BME690 sensor averaging buffer
 */
typedef struct {
    bme690_data_t samples[MAX_SENSOR_SAMPLES];
    uint16_t write_index;
    uint16_t sample_count;
    uint32_t total_samples;  // For sequence tracking
} bme690_sensor_buffer_t;

/**
 * @brief BMV080 sensor buffer (no averaging needed currently)
 */
typedef struct {
    bmv080_data_t samples[MAX_SENSOR_SAMPLES];
    uint16_t write_index;
    uint16_t sample_count;
    uint32_t total_samples;
} bmv080_sensor_buffer_t;

/**
 * @brief Initialize BME690 sensor buffer
 * @param buffer Pointer to buffer to initialize
 */
void bme690_buffer_init(bme690_sensor_buffer_t *buffer);

/**
 * @brief Add sample to BME690 sensor buffer
 * @param buffer Pointer to buffer
 * @param data Pointer to sensor data to add
 */
void bme690_buffer_add(bme690_sensor_buffer_t *buffer, const bme690_data_t *data);

/**
 * @brief Get averaged data from BME690 buffer
 * Only averages the same fields that are currently averaged:
 * temperature, pressure, humidity, iaq, iaq_accuracy, co2_equivalent, breath_voc_equivalent
 * All other fields return latest values
 * @param buffer Pointer to buffer
 * @return Averaged sensor data
 */
bme690_data_t bme690_buffer_get_averaged(const bme690_sensor_buffer_t *buffer);

/**
 * @brief Get latest data from BME690 buffer
 * @param buffer Pointer to buffer
 * @return Latest sensor data
 */
bme690_data_t bme690_buffer_get_latest(const bme690_sensor_buffer_t *buffer);

/**
 * @brief Check if buffer has enough samples for reliable averaging
 * @param buffer Pointer to buffer
 * @param min_samples Minimum number of samples required
 * @return True if buffer has enough samples
 */
bool bme690_buffer_is_ready(const bme690_sensor_buffer_t *buffer, uint16_t min_samples);

/**
 * @brief Initialize BMV080 sensor buffer
 * @param buffer Pointer to buffer to initialize
 */
void bmv080_buffer_init(bmv080_sensor_buffer_t *buffer);

/**
 * @brief Add sample to BMV080 sensor buffer
 * @param buffer Pointer to buffer
 * @param data Pointer to sensor data to add
 */
void bmv080_buffer_add(bmv080_sensor_buffer_t *buffer, const bmv080_data_t *data);

/**
 * @brief Get latest data from BMV080 buffer
 * @param buffer Pointer to buffer
 * @return Latest sensor data
 */
bmv080_data_t bmv080_buffer_get_latest(const bmv080_sensor_buffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_BUFFER_H
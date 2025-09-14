#include "sensor_buffer.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG = "SENSOR_BUFFER";

void bme690_buffer_init(bme690_sensor_buffer_t *buffer) {
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot initialize NULL BME690 buffer");
        return;
    }

    memset(buffer, 0, sizeof(bme690_sensor_buffer_t));
    ESP_LOGI(TAG, "BME690 buffer initialized (max samples: %d)", MAX_SENSOR_SAMPLES);
}

void bme690_buffer_add(bme690_sensor_buffer_t *buffer, const bme690_data_t *data) {
    if (buffer == NULL || data == NULL) {
        ESP_LOGE(TAG, "Cannot add to NULL buffer or NULL data");
        return;
    }

    // Add sample to circular buffer
    buffer->samples[buffer->write_index] = *data;
    buffer->write_index = (buffer->write_index + 1) % MAX_SENSOR_SAMPLES;

    // Update sample count (up to maximum)
    if (buffer->sample_count < MAX_SENSOR_SAMPLES) {
        buffer->sample_count++;
    }

    buffer->total_samples++;

    ESP_LOGD(
        TAG, "Added BME690 sample #%lu (buffer: %d/%d)", (unsigned long)buffer->total_samples, buffer->sample_count, MAX_SENSOR_SAMPLES);
}

bme690_data_t bme690_buffer_get_averaged(const bme690_sensor_buffer_t *buffer) {
    bme690_data_t averaged = {0};

    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot get average from NULL buffer");
        return averaged;
    }

    if (buffer->sample_count == 0) {
        ESP_LOGW(TAG, "Cannot get average from empty buffer");
        return averaged;
    }

    // Get the latest sample as base (for non-averaged fields)
    uint16_t latest_index = (buffer->write_index - 1 + MAX_SENSOR_SAMPLES) % MAX_SENSOR_SAMPLES;
    averaged = buffer->samples[latest_index];

    // Only average the specific fields that were previously averaged:
    // temperature, pressure, humidity, iaq, iaq_accuracy, co2_equivalent, breath_voc_equivalent
    // All other fields (static_iaq, gas_percentage, status flags) use latest values

    double sum_temp = 0.0;
    double sum_pressure = 0.0;
    double sum_humidity = 0.0;
    double sum_iaq = 0.0;
    double sum_iaq_acc = 0.0;
    double sum_co2 = 0.0;
    double sum_voc = 0.0;

    for (uint16_t i = 0; i < buffer->sample_count; i++) {
        const bme690_data_t *sample = &buffer->samples[i];

        sum_temp += sample->temperature;
        sum_pressure += sample->pressure;
        sum_humidity += sample->humidity;
        sum_iaq += sample->iaq;
        sum_iaq_acc += sample->iaq_accuracy;
        sum_co2 += sample->co2_equivalent;
        sum_voc += sample->breath_voc_equivalent;
    }

    uint16_t count = buffer->sample_count;

    // Replace only the averaged fields
    averaged.temperature = (float)(sum_temp / count);
    averaged.pressure = (float)(sum_pressure / count);
    averaged.humidity = (float)(sum_humidity / count);
    averaged.iaq = (float)(sum_iaq / count);
    averaged.iaq_accuracy = (uint8_t)(sum_iaq_acc / count + 0.5); // Round to nearest
    averaged.co2_equivalent = (float)(sum_co2 / count);
    averaged.breath_voc_equivalent = (float)(sum_voc / count);

    // All other fields keep their latest values:
    // - static_iaq (from latest sample)
    // - gas_percentage (from latest sample)
    // - stabilization_status (from latest sample)
    // - run_in_status (from latest sample)
    // - timestamp (from latest sample)

    ESP_LOGD(TAG, "Generated averaged BME690 data from %d samples", count);

    return averaged;
}

bme690_data_t bme690_buffer_get_latest(const bme690_sensor_buffer_t *buffer) {
    bme690_data_t empty = {0};

    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot get latest from NULL buffer");
        return empty;
    }

    if (buffer->sample_count == 0) {
        ESP_LOGW(TAG, "Cannot get latest from empty buffer");
        return empty;
    }

    uint16_t latest_index = (buffer->write_index - 1 + MAX_SENSOR_SAMPLES) % MAX_SENSOR_SAMPLES;
    return buffer->samples[latest_index];
}

bool bme690_buffer_is_ready(const bme690_sensor_buffer_t *buffer, uint16_t min_samples) {
    if (buffer == NULL) {
        return false;
    }

    return buffer->sample_count >= min_samples;
}

void bmv080_buffer_init(bmv080_sensor_buffer_t *buffer) {
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot initialize NULL BMV080 buffer");
        return;
    }

    memset(buffer, 0, sizeof(bmv080_sensor_buffer_t));
    ESP_LOGI(TAG, "BMV080 buffer initialized (max samples: %d)", MAX_SENSOR_SAMPLES);
}

void bmv080_buffer_add(bmv080_sensor_buffer_t *buffer, const bmv080_data_t *data) {
    if (buffer == NULL || data == NULL) {
        ESP_LOGE(TAG, "Cannot add to NULL buffer or NULL data");
        return;
    }

    // Add sample to circular buffer
    buffer->samples[buffer->write_index] = *data;
    buffer->write_index = (buffer->write_index + 1) % MAX_SENSOR_SAMPLES;

    // Update sample count (up to maximum)
    if (buffer->sample_count < MAX_SENSOR_SAMPLES) {
        buffer->sample_count++;
    }

    buffer->total_samples++;

    ESP_LOGD(
        TAG, "Added BMV080 sample #%lu (buffer: %d/%d)", (unsigned long)buffer->total_samples, buffer->sample_count, MAX_SENSOR_SAMPLES);
}

bmv080_data_t bmv080_buffer_get_latest(const bmv080_sensor_buffer_t *buffer) {
    bmv080_data_t empty = {0};

    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot get latest from NULL buffer");
        return empty;
    }

    if (buffer->sample_count == 0) {
        ESP_LOGW(TAG, "Cannot get latest from empty buffer");
        return empty;
    }

    uint16_t latest_index = (buffer->write_index - 1 + MAX_SENSOR_SAMPLES) % MAX_SENSOR_SAMPLES;
    return buffer->samples[latest_index];
}
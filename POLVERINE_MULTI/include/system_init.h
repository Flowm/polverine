#pragma once

/**
 * @brief Initialize power management configuration
 *
 * Configures ESP32 power management for optimal WiFi stability
 * by setting frequency limits and disabling light sleep.
 */
void pm_init(void);

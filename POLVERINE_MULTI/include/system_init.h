#ifndef SYSTEM_INIT_H_
#define SYSTEM_INIT_H_

/**
 * @brief Initialize power management configuration
 *
 * Configures ESP32 power management for optimal WiFi stability
 * by setting frequency limits and disabling light sleep.
 */
void pm_init(void);

#endif /* SYSTEM_INIT_H_ */
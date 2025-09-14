/**
 * @file button_handler.h
 * @brief Handle button press for configuration reset
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize button handler
 * Uses BOOT button (GPIO0) to trigger configuration reset
 */
void button_handler_init(void);

#ifdef __cplusplus
}
#endif

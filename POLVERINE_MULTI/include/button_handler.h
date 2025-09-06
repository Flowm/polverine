/**
 * @file button_handler.h
 * @brief Handle button press for configuration reset
 */

#ifndef BUTTON_HANDLER_H_
#define BUTTON_HANDLER_H_

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

#endif /* BUTTON_HANDLER_H_ */
#ifndef LEDS_H_
#define LEDS_H_

#include <stddef.h>
#include <stdbool.h>

#define STATUS_OK 0
#define STATUS_ERROR 1
#define STATUS_BUSY 2
#define STATUS_TIMEOUT 3
#define STATUS_NOT_FOUND 4
#define STATUS_NOT_SUPPORTED 5
#define STATUS_NOT_INITIALIZED 6
#define STATUS_NOT_CONFIGURED 7
#define STATUS_NOT_READY 8
#define STATUS_NOT_ENABLED 9

void leds_app_start();

extern char stsMain;
extern char stsBMV080;
extern char stsBME690;

#endif

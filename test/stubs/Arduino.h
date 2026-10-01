#pragma once
#include <stdint.h>

// Minimal host stand-ins for the Arduino types used by the original headers.
#define BUILTIN_LED 8
#define INPUT_PULLUP 2
typedef void* QueueHandle_t;

#ifdef __cplusplus
extern "C" {
#endif
void pinMode(int pin, int mode);
int digitalRead(int pin);
#ifdef __cplusplus
}
#endif

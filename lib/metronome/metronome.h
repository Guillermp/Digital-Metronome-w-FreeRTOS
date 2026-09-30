#pragma once
#include "ioMapping_ardustyle.h"
#include "lcd_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

extern volatile double periodMetronome_ms;
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

extern QueueHandle_t display_queue;

uint8_t step_increase(uint8_t current_step, uint8_t array_len);
uint8_t step_decrease(uint8_t current_step, uint8_t array_len);

void initializeMetronomeBPM(void);

void increaseMetronomeRate(void);

void decreaseMetronomeRate(void);

double getMetronomeBPM(void);

#ifdef __cplusplus
}
#endif
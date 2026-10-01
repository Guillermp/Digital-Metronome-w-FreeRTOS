#pragma once
#include "ioMapping_ardustyle.h"
#include "lcd_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

extern QueueHandle_t display_queue;

void initializeMetronomeBPM(void);

void increaseMetronomeRate(void);

void decreaseMetronomeRate(void);

double getMetronomeBPM(void);

double getMetronomePeriod(void);

#ifdef __cplusplus
}
#endif
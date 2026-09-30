#pragma once
#include "ioMapping_ardustyle.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THRESHOLD_DEBOUNCING 5

struct Button {
    uint8_t debounced_button_state;
    uint16_t counter;
    int pinNumber;
    void (*fptr)(void);
};


extern volatile struct Button button_up;
extern volatile struct Button button_down;

void initButtons(void);
void debouncing_logic(uint8_t raw_reading, volatile struct Button* buttonPtr);
void readButtonDebounced(volatile struct Button* buttonPtr);

#ifdef __cplusplus
}
#endif
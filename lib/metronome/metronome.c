#include "metronome.h"

volatile static uint8_t step = 0;
QueueHandle_t display_queue = NULL;

// period to achieve the desired BPMs (check the documentation)
const static double bpm_values[] = {
    60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160
};

double getMetronomeBPM(void) {
    return bpm_values[step];
}

volatile double periodMetronome_ms = 60000.0/bpm_values[0];

void initializeMetronomeBPM(void){
    // set the BPM to the initial value of the table
    cli();
    step = 0;
    sei();
    double bpm = getMetronomeBPM();
    if (xQueueOverwrite(display_queue, &bpm) != pdTRUE) {
    }
    else {
        // Some kind of error logging
    }

}

// Pure logic functions
uint8_t step_increase(uint8_t current_step, uint8_t array_len) {
    current_step++;
    if (current_step >= array_len) current_step = 0;
    return current_step;
}

uint8_t step_decrease(uint8_t current_step, uint8_t array_len) {
    if (current_step == 0)
        return array_len - 1;
    return current_step - 1;
}

void increaseMetronomeRate(void) {
    step = step_increase(step, ARRAY_LEN(bpm_values));
    periodMetronome_ms = 60000.0/bpm_values[step];
    double bpm = getMetronomeBPM();
    if (xQueueOverwrite(display_queue, &bpm) != pdTRUE) {
    }
    else {
        // Some kind of error logging
    }
}

void decreaseMetronomeRate(void) {
    step = step_decrease(step, ARRAY_LEN(bpm_values));
    cli();
    periodMetronome_ms = 60000.0/bpm_values[step];
    sei();
    double bpm = getMetronomeBPM();
    if (xQueueOverwrite(display_queue, &bpm) != pdTRUE) {
    }
    else {
        // Some kind of error logging
    }
}
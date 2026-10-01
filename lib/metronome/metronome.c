#include "metronome.h"

volatile static uint8_t step = 0U;
QueueHandle_t display_queue = NULL;

// period to achieve the desired BPMs (check the documentation)
const static double bpm_min_value = 40.0;
const static double bpm_max_value = 250.0;
const static double bpm_resolution = 10.0;

const static uint8_t step_max = (bpm_max_value - bpm_min_value)/bpm_resolution;

double getMetronomeBPM(void) {
    return bpm_min_value + bpm_resolution*step;
}

double getMetronomePeriod(void) {
    return 60000.0/getMetronomeBPM();
}
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
uint8_t static step_increase(uint8_t current_step) {
    current_step++;
    if (current_step >= step_max) current_step = 0;
    return current_step;
}

uint8_t static step_decrease(uint8_t current_step) {
    if (current_step == 0)
        return step_max - 1;
    return current_step - 1;
}

void increaseMetronomeRate(void) {
    step = step_increase(step);
    double bpm = getMetronomeBPM();
    if (xQueueOverwrite(display_queue, &bpm) != pdTRUE) {
    }
    else {
        // Some kind of error logging
    }
}

void decreaseMetronomeRate(void) {
    step = step_decrease(step);
    double bpm = getMetronomeBPM();
    if (xQueueOverwrite(display_queue, &bpm) != pdTRUE) {
    }
    else {
        // Some kind of error logging
    }
}
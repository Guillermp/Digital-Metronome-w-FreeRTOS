#include "button.h"
#include "metronome.h"


volatile struct Button button_up = {0, THRESHOLD_DEBOUNCING, pinButtonUp, &increaseMetronomeRate};
volatile struct Button button_down = {0, THRESHOLD_DEBOUNCING, pinButtonDown, &decreaseMetronomeRate};


void initButtons(void) {
    pinMode(pinButtonUp, INPUT_PULLUP); // set pin direction to input
    button_up.debounced_button_state = digitalRead(pinButtonUp);

    pinMode(pinButtonDown, INPUT_PULLUP); // set pin direction to input
    button_down.debounced_button_state = digitalRead(pinButtonDown);

    button_up.counter = THRESHOLD_DEBOUNCING;
    button_down.counter = THRESHOLD_DEBOUNCING;
}


void debouncing_logic(uint8_t raw_reading, volatile struct Button* buttonPtr) {

    // If raw reading same as debounced button value, nothing has changed, reset the counter
     if (raw_reading == buttonPtr->debounced_button_state) {
        buttonPtr->counter = THRESHOLD_DEBOUNCING;
    }
    else {
        //Decrement the counter
        buttonPtr->counter -=  1;
        
        if (buttonPtr->counter == 0) {
           
            //buttonPtr->button_pressed = 1;
            buttonPtr->debounced_button_state = raw_reading;
            buttonPtr->counter = THRESHOLD_DEBOUNCING;

            //If the debounced value is pressed
            if (buttonPtr->debounced_button_state == 1) {
                buttonPtr->fptr(); // Increase or decrese the BPM of the metronome
            }

        }
        
    }

}

void readButtonDebounced(volatile struct Button* buttonPtr) {
    uint8_t raw_reading = digitalRead(buttonPtr->pinNumber);
    debouncing_logic(raw_reading, buttonPtr);
    
}
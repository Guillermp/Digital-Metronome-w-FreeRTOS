#include "led.h"

void initLED(void) {
    pinMode(pinLed, OUTPUT);  // set pin direction to output
}

void turnOnLED(void) {
    digitalWrite(pinLed, HIGH);  // drive pin high
}

void turnOffLED(void) {
    digitalWrite(pinLed, LOW); // drive pin low
}
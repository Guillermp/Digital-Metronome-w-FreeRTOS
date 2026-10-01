#ifndef ARDUINO
#include <Arduino.h>
#include <metronome.h>

// Hardware and BPM callbacks are outside the debounce unit under test.
// Each test installs counting callbacks on the real production Button objects.
extern "C" void pinMode(int, int) {}
extern "C" int digitalRead(int) { return 1; }
extern "C" void increaseMetronomeRate(void) {}
extern "C" void decreaseMetronomeRate(void) {}
#endif

#include <unity.h>
#include <button.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

// INPUT_PULLUP: grounding the input presses the button.
static const uint8_t PRESSED = 0;
static const uint8_t RELEASED = 1;

static unsigned int up_actions;
static unsigned int down_actions;
static void on_up(void) { ++up_actions; }
static void on_down(void) { ++down_actions; }


void setUp(void) {
    up_actions = 0;
    down_actions = 0;
    button_up.debounced_button_state = RELEASED;
    button_up.counter = THRESHOLD_DEBOUNCING;
    button_up.fptr = on_up;
    button_down.debounced_button_state = RELEASED;
    button_down.counter = THRESHOLD_DEBOUNCING;
    button_down.fptr = on_down;
}

void tearDown(void) {}

static void sample(uint8_t value, volatile struct Button* button, int count) {
    for (int i = 0; i < count; ++i) {
        debouncing_logic(value, button);
    }
}

// A stable LOW triggers the callback; a stable HIGH releases the button.
// Both transitions must meet the debounce threshold.
void test_debouncing_logic_when_buttonUp_pressed(void) {
    for (int i = 0; i < THRESHOLD_DEBOUNCING - 1; ++i) {
        debouncing_logic(PRESSED, &button_up);
        TEST_ASSERT_EQUAL_UINT8(RELEASED, button_up.debounced_button_state);
        TEST_ASSERT_EQUAL_UINT(0, up_actions);
    }
    debouncing_logic(PRESSED, &button_up);
    TEST_ASSERT_EQUAL_UINT8(PRESSED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
    TEST_ASSERT_EQUAL_UINT(THRESHOLD_DEBOUNCING, button_up.counter);
}

void test_debouncing_logic_when_buttonDown_pressed(void) {
    for (int i = 0; i < THRESHOLD_DEBOUNCING - 1; ++i) {
        debouncing_logic(PRESSED, &button_down);
        TEST_ASSERT_EQUAL_UINT8(RELEASED, button_down.debounced_button_state);
        TEST_ASSERT_EQUAL_UINT(0, down_actions);
    }
    debouncing_logic(PRESSED, &button_down);
    TEST_ASSERT_EQUAL_UINT8(PRESSED, button_down.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(1, down_actions);
}

void test_debouncing_logic_releasing_after_pressed(void) {
    button_down.debounced_button_state = PRESSED;
    for (int i = 0; i < THRESHOLD_DEBOUNCING - 1; ++i) {
        debouncing_logic(RELEASED, &button_down);
        TEST_ASSERT_EQUAL_UINT8(PRESSED, button_down.debounced_button_state);
    }
    debouncing_logic(RELEASED, &button_down);
    TEST_ASSERT_EQUAL_UINT8(RELEASED, button_down.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(0, down_actions);
}

void test_debouncing_is_count_reset(void) {
    for (uint8_t state = 0; state <= 1; ++state) {
        button_up.debounced_button_state = state;
        button_up.counter = 2;
        debouncing_logic(state, &button_up);
        TEST_ASSERT_EQUAL_UINT(THRESHOLD_DEBOUNCING, button_up.counter);
    }
    TEST_ASSERT_EQUAL_UINT(0, up_actions);
}

void test_bounce_requires_a_new_full_sequence(void) {
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING - 1);
    debouncing_logic(RELEASED, &button_up);
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING - 1);
    TEST_ASSERT_EQUAL_UINT8(RELEASED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(0, up_actions);
    debouncing_logic(PRESSED, &button_up);
    TEST_ASSERT_EQUAL_UINT8(PRESSED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
}

void test_held_button_fires_only_once(void) {
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING * 4);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
}

void test_release_bounce_does_not_retrigger(void) {
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING);
    sample(RELEASED, &button_up, THRESHOLD_DEBOUNCING - 1);
    debouncing_logic(PRESSED, &button_up);
    sample(RELEASED, &button_up, THRESHOLD_DEBOUNCING - 1);
    TEST_ASSERT_EQUAL_UINT8(PRESSED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
    debouncing_logic(RELEASED, &button_up);
    TEST_ASSERT_EQUAL_UINT8(RELEASED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
}

void test_second_press_fires_again(void) {
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING);
    sample(RELEASED, &button_up, THRESHOLD_DEBOUNCING);
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING);
    TEST_ASSERT_EQUAL_UINT(2, up_actions);
}

void test_buttons_are_independent(void) {
    sample(PRESSED, &button_up, THRESHOLD_DEBOUNCING - 1);
    sample(PRESSED, &button_down, THRESHOLD_DEBOUNCING);
    TEST_ASSERT_EQUAL_UINT8(RELEASED, button_up.debounced_button_state);
    TEST_ASSERT_EQUAL_UINT(0, up_actions);
    TEST_ASSERT_EQUAL_UINT(1, down_actions);
    debouncing_logic(PRESSED, &button_up);
    TEST_ASSERT_EQUAL_UINT(1, up_actions);
    TEST_ASSERT_EQUAL_UINT(1, down_actions);
}

static int run_tests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_debouncing_logic_when_buttonUp_pressed);
    RUN_TEST(test_debouncing_logic_when_buttonDown_pressed);
    RUN_TEST(test_debouncing_logic_releasing_after_pressed);
    RUN_TEST(test_debouncing_is_count_reset);
    RUN_TEST(test_bounce_requires_a_new_full_sequence);
    RUN_TEST(test_held_button_fires_only_once);
    RUN_TEST(test_release_bounce_does_not_retrigger);
    RUN_TEST(test_second_press_fires_again);
    RUN_TEST(test_buttons_are_independent);
    return UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    run_tests();
}
void loop() {}
#else
int main() { return run_tests(); }
#endif

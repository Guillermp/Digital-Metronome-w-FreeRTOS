#include <unity.h>
#include <metronome.h>
#include <button.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

// ---- Tests for the increase and decrease BPM logic ---- //
void test_increase_normal_case(void) {
    // Property: increasing from the middle just moves forward by 1
    TEST_ASSERT_EQUAL(4, step_increase(3, 11));
}

void test_increase_wraps_at_top(void) {
    // Property: incrementing past the last index wraps to 0
    TEST_ASSERT_EQUAL(0, step_increase(10, 11));
}

void test_decrease_normal_case(void) {
    // Property: decreasing from the middle just moves back by 1
    TEST_ASSERT_EQUAL(2, step_decrease(3, 11));
}

void test_decrease_wraps_at_bottom(void) {
    // Property: decrementing below 0 wraps to the last valid index
    TEST_ASSERT_EQUAL(10, step_decrease(0, 11));
}

void test_increase_decrease_are_inverses(void) {
    // Property: increase then decrease returns you to where you started
    uint8_t start = 5;
    uint8_t after_inc = step_increase(start, 11);
    uint8_t after_dec = step_decrease(after_inc, 11);
    TEST_ASSERT_EQUAL(start, after_dec);
}

void test_result_always_in_bounds(void) {
    // Property: no matter the starting point, result is always a valid array index
    for (uint8_t i = 0; i < 11; i++) {
        TEST_ASSERT_TRUE(step_increase(i, 11) < 11);
        TEST_ASSERT_TRUE(step_decrease(i, 11) < 11);
    }
}

void test_works_with_different_array_lengths(void) {
    // Property: logic isn't hardcoded to 11 — works for any size
    TEST_ASSERT_EQUAL(0, step_increase(2, 3));   // wraps at len=3
    TEST_ASSERT_EQUAL(2, step_decrease(0, 3));   // wraps at len=3
}

// Test for the debouncing logic

void test_debouncing_logic_when_buttonUp_pressed(void) {
    // Check the behaviour when the button is pressed continuously
    uint8_t raw_reading = 1;
    for (int i = 0; i < THRESHOLD_DEBOUNCING-1; i++){
        debouncing_logic(raw_reading, &button_up);
        TEST_ASSERT_EQUAL_UINT8(0, button_up.debounced_button_state);

    }
    debouncing_logic(raw_reading, &button_up);
    TEST_ASSERT_TRUE(button_up.debounced_button_state == 1);
}

void test_debouncing_logic_when_buttonDown_pressed(void) {
    // Check the behaviour when the button is pressed continuously
    uint8_t raw_reading = 1;
    for (int i = 0; i < THRESHOLD_DEBOUNCING-1; i++){
        debouncing_logic(raw_reading, &button_down);
        TEST_ASSERT_EQUAL_UINT8(0, button_down.debounced_button_state);

    }
    debouncing_logic(raw_reading, &button_down);
    TEST_ASSERT_TRUE(button_down.debounced_button_state == 1);
    
}

void test_debouncing_logic_releasing_after_pressed(void) {
    // Check the behaviour when the button is released after being pressed (debounced pressed)
    button_up.debounced_button_state = 1; // We mock that the button press was acknowledged 

    // Button released
    uint8_t raw_reading = 0;
    for (int i = 0; i < THRESHOLD_DEBOUNCING-1; i++){
        debouncing_logic(raw_reading, &button_down);
        TEST_ASSERT_EQUAL_UINT8(1, button_down.debounced_button_state);

    }
    debouncing_logic(raw_reading, &button_down);
    TEST_ASSERT_TRUE(button_down.debounced_button_state == 0);
}

void test_debouncing_is_count_reset(void) {
    button_up.counter = 2;
    // Did the count reset instantly after the debounced state is equal to the current reading?
    button_up.debounced_button_state = 1; // We mock that the button press was acknowledged 
    uint8_t raw_reading = 1;
    debouncing_logic(raw_reading, &button_up);
    TEST_ASSERT_TRUE(button_up.counter == THRESHOLD_DEBOUNCING);

    button_up.counter = 2;
    // Did the count reset instantly after the debounced state is equal to the current reading?
    button_up.debounced_button_state = 0; // We mock that the button press was acknowledged 
    raw_reading = 0;
    debouncing_logic(raw_reading, &button_up);
    TEST_ASSERT_TRUE(button_up.counter == THRESHOLD_DEBOUNCING);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_increase_normal_case);
    RUN_TEST(test_increase_wraps_at_top);

    RUN_TEST(test_decrease_normal_case);
    RUN_TEST(test_decrease_wraps_at_bottom);

    RUN_TEST(test_increase_decrease_are_inverses);

    RUN_TEST(test_result_always_in_bounds);

    RUN_TEST(test_works_with_different_array_lengths);

    // Tests for debouncing logic
    RUN_TEST(test_debouncing_logic_when_buttonUp_pressed);
    RUN_TEST(test_debouncing_logic_when_buttonDown_pressed);
    RUN_TEST(test_debouncing_logic_releasing_after_pressed);
    RUN_TEST(test_debouncing_is_count_reset);

    UNITY_END();
}

void loop() {}
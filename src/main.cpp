#include "led.h"
#include "button.h"
#include "metronome.h"
#include "buzzer.h"

SemaphoreHandle_t metronome_mutex;

// Read the button periodically
void readButtons_task(void * pvParameters) {
  for (;;) {
  xSemaphoreTake(metronome_mutex, portMAX_DELAY);
  readButtonDebounced(&button_up);
  readButtonDebounced(&button_down);
  xSemaphoreGive(metronome_mutex);
  vTaskDelay(5 / portTICK_PERIOD_MS);
  }
  
}

TaskHandle_t ledTaskHandle;

void led_task(void *pvParameters) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        turnOnLED();
        vTaskDelay(pdMS_TO_TICKS(30));
        turnOffLED();
    }
}


// Metronome
void buzzer_task(void * pvParameters) {
  for (;;) {
  xSemaphoreTake(metronome_mutex, portMAX_DELAY);
  double period_ms = periodMetronome_ms;
  xSemaphoreGive(metronome_mutex);
  xTaskNotifyGive(ledTaskHandle);
  tone(pinBuzzer, 1000, 30);
  vTaskDelay(period_ms / portTICK_PERIOD_MS);
  }
}

void updateDisplay_task(void * pvParameters) {
  double bpm;
  

  for( ;; ) 
    {
        // Blocks indefinitely -- no polling, no wasted CPU.
        if (xQueueReceive(display_queue, &bpm, portMAX_DELAY) == pdTRUE) {
            lcd_set_cursor(0, 0);
            lcd_print("BPM: ");
            lcd_print_double(bpm,0);
            lcd_print(" ");
        }
    }
}
void setup() {

  display_queue = xQueueCreate(1, sizeof(double));
  configASSERT(display_queue != NULL);

  metronome_mutex = xSemaphoreCreateMutex();
  configASSERT(metronome_mutex != NULL);

  // Setup
  init();
  initButtons();
  initLED();
  initBuzzer ();
  lcd_init(); // SDA on pin 4 and SCL on pin 6
  initializeMetronomeBPM();

  xTaskCreate(readButtons_task,   "button_task",   4096, NULL, 3, NULL);
  xTaskCreate(led_task, "led_task", 2048, NULL, 2, &ledTaskHandle);
  xTaskCreate(buzzer_task,   "buzzer_task",   4096, NULL, 2, NULL);
  xTaskCreate(updateDisplay_task, "updateDisplay_task", 4096, NULL, 1, NULL);
  //Serial.begin(9600);

}

void loop(){}
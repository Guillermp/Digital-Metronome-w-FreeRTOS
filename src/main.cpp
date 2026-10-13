#include "led.h"
#include "button.h"
#include "metronome.h"
#include "buzzer.h"
#include "riffWeb.h"

SemaphoreHandle_t metronome_mutex;

QueueHandle_t beatsPerMeasure_queue;

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


void buzzer_task(void * pvParameters) {
  uint32_t beatsPerMeasure = Measures::fourBeatsPerMeasure;
  uint32_t beat = 1;
  for (;;) {
  uint32_t incomingBeatsPerMeasure;
  xSemaphoreTake(metronome_mutex, portMAX_DELAY);
  double period_ms = getMetronomePeriod();
  xSemaphoreGive(metronome_mutex);
  xTaskNotifyGive(ledTaskHandle);

  // To have the required beats per measure

  // Check if there is any update in the beats per measure
  if (xQueueReceive(beatsPerMeasure_queue, &incomingBeatsPerMeasure, 0) == pdTRUE) {
            beatsPerMeasure = incomingBeatsPerMeasure;
        }
  
  beat++;
  if (beat >= beatsPerMeasure) {
    tone(pinBuzzer, 600, 30);
    beat = 0;

  } else {
    tone(pinBuzzer, 500, 30);
  }

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

  beatsPerMeasure_queue = xQueueCreate(1, sizeof(uint32_t));
  configASSERT(beatsPerMeasure_queue != NULL);

  // Setup
  init();
  initButtons();
  initLED();
  initBuzzer ();
  lcd_init(); // SDA on pin 4 and SCL on pin 6
  initializeMetronomeBPM();
  Serial.begin(115200);

  RiffWeb::begin();

  xTaskCreate(readButtons_task,   "button_task",   4096, NULL, 3, NULL);
  xTaskCreate(led_task, "led_task", 2048, NULL, 2, &ledTaskHandle);
  xTaskCreate(buzzer_task,   "buzzer_task",   4096, NULL, 2, NULL);
  xTaskCreate(updateDisplay_task, "updateDisplay_task", 4096, NULL, 1, NULL);
  

}

void loop(){
  RiffWeb::service();
  delay(2);
}
#include "hardware.h"
#include "tasks.h"

void setup() {
  // 1) Initialize hardware (LCD, RTC, servos, pins, etc.)
  hardwareInit();

  // 2) Create tasks
  //    - We'll pin the tasks as in the previous example, or you can omit the last param for "auto" core assignment.
  xTaskCreatePinnedToCore(TaskCheckButtons, "CheckButtons", 2048, NULL, 2, NULL, 0);

  // Mode tasks
  xTaskCreatePinnedToCore(TaskMode0, "Mode0", 4096, NULL, 1, &handleMode0, 1);
  xTaskCreatePinnedToCore(TaskMode1, "Mode1", 4096, NULL, 1, &handleMode1, 1);
  xTaskCreatePinnedToCore(TaskMode2, "Mode2", 4096, NULL, 1, &handleMode2, 1);

  // mode=0 initially, so we want only TaskMode0 running.
  // Suspend TaskMode1 & TaskMode2
  vTaskSuspend(handleMode1);
  vTaskSuspend(handleMode2);

  Serial.println("Setup complete. Mode0 task running, others suspended.");
}

void loop() {
  // Minimal or empty. FreeRTOS tasks do the heavy lifting.
  vTaskDelay(1);
}
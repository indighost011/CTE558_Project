#include "tasks.h"
#include "hardware.h"  // So we can use 'mode', the LCD, etc.

// Task handles
TaskHandle_t handleMode0 = NULL;
TaskHandle_t handleMode1 = NULL;
TaskHandle_t handleMode2 = NULL;

/**
 * TaskCheckButtons:
 *   Continuously reads BUTTON_PIN_1 to cycle through modes (0->1->2->0).
 *   Suspends the old mode task and resumes the new one.
 */
void TaskCheckButtons(void *pvParams) {
  static int lastMode = 0;
  for (;;) {
    if (digitalRead(BUTTON_PIN_1) == LOW) {
      vTaskDelay(200 / portTICK_PERIOD_MS); // debounce
      mode = (mode + 1) % 3;
      Serial.printf("Mode changed to: %d\n", mode);

      // Suspend the old mode's task
      if (lastMode == 0 && handleMode0) vTaskSuspend(handleMode0);
      if (lastMode == 1 && handleMode1) vTaskSuspend(handleMode1);
      if (lastMode == 2 && handleMode2) vTaskSuspend(handleMode2);

      // Resume the new mode's task
      if (mode == 0 && handleMode0) vTaskResume(handleMode0);
      if (mode == 1 && handleMode1) vTaskResume(handleMode1);
      if (mode == 2 && handleMode2) vTaskResume(handleMode2);

      lastMode = mode;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/**
 * TaskMode0 (Normal Dispensing & RTC Display):
 *   Runs in a loop, but only if this task is not suspended.
 */
void TaskMode0(void *pvParams) {
  for (;;) {
    // Clear & display time
    lcd.clear();
    displayRTCDateTime();

    DateTime now = rtc.now();
    // Automatic dispensing checks
    for (int i = 0; i < 3; i++) {
      bool executeServo = false;
      if (frequencyChoices[i] == 1) {
        // Daily
        if (now.hour() == drugHours[i] && now.minute() == drugMinutes[i]) {
          executeServo = true;
        }
      } 
      else if (frequencyChoices[i] == 2) {
        // Weekly
        if (now.dayOfTheWeek() == drugDays[i] &&
            now.hour()         == drugHours[i] &&
            now.minute()       == drugMinutes[i]) {
          executeServo = true;
        }
      } 
      else if (frequencyChoices[i] == 3) {
        // Monthly
        if (now.day()    == drugDays[i] &&
            now.hour()   == drugHours[i] &&
            now.minute() == drugMinutes[i]) {
          executeServo = true;
        }
      }

      if (executeServo && !servoActionExecuted[i]) {
        dispenseDrug(i);
      }
    }
    
    // Reset servo flags at the start of each hour
    if (now.minute() == 0 && now.second() == 0) {
      for (int i = 0; i < 3; i++) {
        servoActionExecuted[i] = false;
      }
    }

    // Update once per second, or as you prefer
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/**
 * TaskMode1 (Time-Setting via Joystick):
 *   Only runs when mode=1 is active (not suspended).
 */
void TaskMode1(void *pvParams) {
  for (;;) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode 1: Time");
    lcd.setCursor(0, 1);
    lcd.print("Setting...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // The blocking time-set function
    setTimeUsingJoystick();  // sets mode=0 when done

    // small pause
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/**
 * TaskMode2 (Drug Scheduling):
 *   Only runs when mode=2 is active.
 */
void TaskMode2(void *pvParams) {
  for (;;) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode 2: Sched");
    lcd.setCursor(0, 1);
    lcd.print("Drug Config...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // The blocking scheduling function
    Scheduling();  // sets mode=0 when done

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

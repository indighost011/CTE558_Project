#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP32Servo.h>

// ------------------ Pin Definitions ------------------
extern const int JOY_VERT_PIN;
extern const int JOY_HORZ_PIN;
extern const int DRUG_BUTTON;

extern const int BUTTON_PIN_1;
extern const int BUTTON_PIN_2;
extern const int BUTTON_PIN_3;

extern const int LCD_SDA_PIN;
extern const int LCD_SCL_PIN;
extern const int LCD_ADDRESS;

extern const int SERVO_PINS[3];

// ------------------ Hardware Objects ------------------
extern LiquidCrystal_I2C lcd;   // The LCD
extern RTC_DS3231 rtc;          // The RTC

extern Servo servos[3];
extern bool servoActionExecuted[3];

// ------------------ Drug Data ------------------
extern String drugNames[3];
extern int frequencyChoices[3];
extern int pillCounts[3];
extern int drugDays[3];
extern int drugHours[3];
extern int drugMinutes[3];

// For scheduling, etc.
extern int tempFrequency;
extern int tempDay;
extern int tempHour;
extern int tempMinute;
extern int tempPillCount;

extern int currentIndex;
extern int currentField;
extern String fieldNames[5];

// Shared mode variable
extern volatile int mode; 

// ------------------ Functions ------------------
void hardwareInit();
void displayRTCDateTime();
void dispenseDrug(int index);

// If you want them here, you can also put the joystick & scheduling helpers here:
void handleJoystick(int xValue, int yValue);
void loadTempValues();
void saveDrugDetails();
void updateLCD2();
void changeDrugIndex();
void setTimeUsingJoystick();
void Scheduling();

#endif

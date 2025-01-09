#include "hardware.h"

// ------------------ Pin Definitions ------------------
const int JOY_VERT_PIN  = 34;
const int JOY_HORZ_PIN  = 35;
const int DRUG_BUTTON   = 32;

const int BUTTON_PIN_1  = 2;
const int BUTTON_PIN_2  = 0;
const int BUTTON_PIN_3  = 4;

const int LCD_SDA_PIN   = 21;
const int LCD_SCL_PIN   = 22;
const int LCD_ADDRESS   = 0x27;

const int SERVO_PINS[3] = {18, 5, 17};

// ------------------ Hardware Objects ------------------
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
RTC_DS3231 rtc;

Servo servos[3];
bool servoActionExecuted[3] = {false, false, false};

// ------------------ Drug Data ------------------
String drugNames[3]       = {"Drug1", "Drug2", "Drug3"};
int frequencyChoices[3]    = {0, 0, 0};
int pillCounts[3]          = {0, 0, 0};
int drugDays[3]            = {0, 0, 0};
int drugHours[3]           = {0, 0, 0};
int drugMinutes[3]         = {0, 0, 0};

int tempFrequency          = 1;
int tempDay                = 0;
int tempHour               = 0;
int tempMinute             = 0;
int tempPillCount          = 1;

int currentIndex           = 0;
int currentField           = 0;
String fieldNames[5]       = {"Freq", "Day", "Hour", "Min", "Pills"};

volatile int mode          = 0; // 0=normal,1=time-set,2=scheduling

// -------------------------------------------------------
//            Implementation of Hardware Functions
// -------------------------------------------------------
void hardwareInit()
{
  // Serial for debug
  Serial.begin(115200);

  // Initialize I2C for LCD & RTC
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  
  // LCD
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    Serial.println("RTC initialization failed!");
    lcd.print("RTC Error");
    // could block here if needed
    while (1) { delay(1000); }
  }

  // Servos
  for (int i = 0; i < 3; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(90); // initial position
  }

  // Pins
  pinMode(JOY_VERT_PIN,  INPUT);
  pinMode(JOY_HORZ_PIN,  INPUT);
  pinMode(DRUG_BUTTON,   INPUT_PULLUP);
  pinMode(BUTTON_PIN_1,  INPUT_PULLUP);
  pinMode(BUTTON_PIN_2,  INPUT_PULLUP);
  pinMode(BUTTON_PIN_3,  INPUT_PULLUP);

  Serial.println("hardwareInit: done.");
}

void displayRTCDateTime() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.printf("%02d-%02d-%04d", now.day(), now.month(), now.year());
  lcd.setCursor(0, 1);
  lcd.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
}

void dispenseDrug(int index) {
  if (index < 0 || index >= 3) {
    Serial.println("Invalid drug index.");
    return;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing drug:");
  lcd.setCursor(0, 1);
  lcd.print(drugNames[index]);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (int i = 0; i < pillCounts[index]; i++) {
    servos[index].write(0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    servos[index].write(90);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  servoActionExecuted[index] = true;
  lcd.clear();
}

// -------------------------------------------------------
//               Other "Helper" Functions
// -------------------------------------------------------
void handleJoystick(int xValue, int yValue) {
  // Horizontal for field navigation
  if (xValue == 4095) {
    currentField = (currentField - 1 + 5) % 5;
  } else if (xValue == 0) {
    currentField = (currentField + 1) % 5;
  }

  // Vertical for increment/decrement
  if (yValue == 0) {
    // Decrement
    switch (currentField) {
      case 0: tempFrequency = constrain(tempFrequency - 1, 1, 3); break;
      case 1: tempDay       = constrain(tempDay - 1, 0, 31);      break;
      case 2: tempHour      = constrain(tempHour - 1, 0, 23);     break;
      case 3: tempMinute    = constrain(tempMinute - 1, 0, 59);   break;
      case 4: tempPillCount = constrain(tempPillCount - 1, 1, 10);break;
    }
  } else if (yValue == 4095) {
    // Increment
    switch (currentField) {
      case 0: tempFrequency = constrain(tempFrequency + 1, 1, 3); break;
      case 1: tempDay       = constrain(tempDay + 1, 0, 31);      break;
      case 2: tempHour      = constrain(tempHour + 1, 0, 23);     break;
      case 3: tempMinute    = constrain(tempMinute + 1, 0, 59);   break;
      case 4: tempPillCount = constrain(tempPillCount + 1, 1, 10);break;
    }
  }
}

void loadTempValues() {
  tempFrequency = frequencyChoices[currentIndex];
  tempDay       = drugDays[currentIndex];
  tempHour      = drugHours[currentIndex];
  tempMinute    = drugMinutes[currentIndex];
  tempPillCount = pillCounts[currentIndex];
}

void saveDrugDetails() {
  frequencyChoices[currentIndex] = tempFrequency;
  drugDays[currentIndex]        = tempDay;
  drugHours[currentIndex]       = tempHour;
  drugMinutes[currentIndex]     = tempMinute;
  pillCounts[currentIndex]      = tempPillCount;

  Serial.print("Saved => Drug #");
  Serial.print(currentIndex + 1);
  Serial.print(": Freq=");
  Serial.print(tempFrequency);
  Serial.print(", Day=");
  Serial.print(tempDay);
  Serial.print(", Hour=");
  Serial.print(tempHour);
  Serial.print(", Min=");
  Serial.print(tempMinute);
  Serial.print(", Pills=");
  Serial.println(tempPillCount);
}

void updateLCD2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Drug ");
  lcd.print(currentIndex + 1);

  lcd.setCursor(0, 1);
  lcd.print(fieldNames[currentField]);
  lcd.print(": ");
  switch (currentField) {
    case 0: lcd.print(tempFrequency); break;
    case 1: lcd.print(tempDay);       break;
    case 2: lcd.print(tempHour);      break;
    case 3: lcd.print(tempMinute);    break;
    case 4: lcd.print(tempPillCount); break;
  }
}

void changeDrugIndex() {
  currentIndex = (currentIndex + 1) % 3;
  loadTempValues();
  Serial.print("Switched to Drug ");
  Serial.println(currentIndex + 1);
}

// The "blocking" time-set function
void setTimeUsingJoystick() {
  DateTime currentTime = rtc.now();
  int year   = currentTime.year();
  int month  = currentTime.month();
  int day    = currentTime.day();
  int hour   = currentTime.hour();
  int minute = currentTime.minute();
  int selectedField = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Time:");

  while (digitalRead(BUTTON_PIN_2) == HIGH &&
         digitalRead(BUTTON_PIN_3) == HIGH &&
         digitalRead(BUTTON_PIN_1) == HIGH)
  {
    // Display date/time
    lcd.setCursor(0, 1);
    lcd.print("                "); // clear line
    lcd.setCursor(0, 1);
    lcd.printf("%04d/%02d/%02d %02d:%02d", 
               year, month, day, hour, minute);

    // Indicate current field
    switch (selectedField) {
      case 0: lcd.setCursor(0, 1);  lcd.print(">>"); break; // Year
      case 1: lcd.setCursor(5, 1);  lcd.print(">>"); break; // Month
      case 2: lcd.setCursor(8, 1);  lcd.print(">>"); break; // Day
      case 3: lcd.setCursor(11, 1); lcd.print(">>"); break; // Hour
      case 4: lcd.setCursor(14, 1); lcd.print(">>"); break; // Minute
    }

    // Joystick
    int vertical   = analogRead(JOY_VERT_PIN);
    int horizontal = analogRead(JOY_HORZ_PIN);

    // Increment or decrement the selected field
    if (vertical == 4095) {
      if (selectedField == 0) year++;
      else if (selectedField == 1) month = (month % 12) + 1;
      else if (selectedField == 2) day   = (day % 31) + 1;
      else if (selectedField == 3) hour  = (hour + 1) % 24;
      else if (selectedField == 4) minute= (minute + 1) % 60;
    }
    else if (vertical == 0) {
      if (selectedField == 0) year--;
      else if (selectedField == 1) month  = (month == 1 ? 12 : month - 1);
      else if (selectedField == 2) day    = (day == 1 ? 31 : day - 1);
      else if (selectedField == 3) hour   = (hour - 1 + 24) % 24;
      else if (selectedField == 4) minute = (minute - 1 + 60) % 60;
    }

    // Move between fields
    if (horizontal == 4095) { // left
      selectedField = (selectedField - 1 + 5) % 5;
    } else if (horizontal == 0) { // right
      selectedField = (selectedField + 1) % 5;
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  // After editing
  if (digitalRead(BUTTON_PIN_2) == LOW) {
    // Save
    rtc.adjust(DateTime(year, month, day, hour, minute, 0));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time Updated!");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    lcd.clear();
    mode = 0;
  } 
  else if (digitalRead(BUTTON_PIN_3) == LOW) {
    // Cancel
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Cancelled");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    lcd.clear();
    mode = 0;
  }
}

void Scheduling() {
  // blocking loop until Button 2 or 3 is pressed
  while (digitalRead(BUTTON_PIN_2) == HIGH &&
         digitalRead(BUTTON_PIN_3) == HIGH)
  {
    int yValue = analogRead(JOY_VERT_PIN);
    int xValue = analogRead(JOY_HORZ_PIN);

    handleJoystick(xValue, yValue);

    if (digitalRead(DRUG_BUTTON) == LOW) {
      changeDrugIndex();
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    // Update the LCD
    updateLCD2();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  // If Button 2 is pressed, save
  if (digitalRead(BUTTON_PIN_2) == LOW) {
    saveDrugDetails();
  }
  // Exit scheduling
  mode = 0;
}


#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

// We need to declare TaskHandle_t for each mode
extern TaskHandle_t handleMode0;
extern TaskHandle_t handleMode1;
extern TaskHandle_t handleMode2;

// Mode tasks
void TaskCheckButtons(void *pvParams);
void TaskMode0       (void *pvParams);
void TaskMode1       (void *pvParams);
void TaskMode2       (void *pvParams);

#endif

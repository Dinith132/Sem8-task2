#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

enum LightState { NS_GREEN_PHASE, NS_YELLOW_PHASE, EW_GREEN_PHASE, EW_YELLOW_PHASE };

extern LightState currentState;
extern unsigned long stateStartTime;
extern unsigned long currentGreenDuration;

extern volatile int northCount;
extern volatile int southCount;
extern volatile int eastCount;
extern volatile int westCount;

#endif

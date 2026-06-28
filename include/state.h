#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

enum LightState { NS_GREEN_PHASE, NS_YELLOW_PHASE, EW_GREEN_PHASE, EW_YELLOW_PHASE };

extern LightState currentState;
extern unsigned long stateStartTime;
extern unsigned long currentGreenDuration;

extern int northCount;
extern int southCount;
extern int eastCount;
extern int westCount;

extern bool lastNorthIn;
extern bool lastNorthOut;
extern bool lastSouthIn;
extern bool lastSouthOut;
extern bool lastEastIn;
extern bool lastEastOut;
extern bool lastWestIn;
extern bool lastWestOut;

#endif

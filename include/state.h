#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

enum Lane { LANE_A = 0, LANE_B = 1, LANE_COUNT = 2 };

enum LightState {
  LANE_A_GREEN,
  LANE_A_YELLOW,
  LANE_B_GREEN,
  LANE_B_YELLOW,
  DECIDE_PHASE
};

extern LightState currentState;
extern unsigned long stateStartTime;
extern unsigned long currentGreenDuration;
extern unsigned long lastGreenTimestamp[LANE_COUNT];

extern volatile int northCount;
extern volatile int southCount;
extern volatile int eastCount;
extern volatile int westCount;

#endif

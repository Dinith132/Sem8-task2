#include "state.h"
#include "config.h"

LightState currentState = LANE_A_GREEN;
unsigned long stateStartTime = 0;
unsigned long currentGreenDuration = BASE_GREEN_TIME;
unsigned long lastGreenTimestamp[LANE_COUNT] = {0, 0};

volatile int northCount = 0;
volatile int southCount = 0;
volatile int eastCount = 0;
volatile int westCount = 0;

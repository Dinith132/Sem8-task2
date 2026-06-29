#include "state.h"
#include "config.h"

LightState currentState = NS_GREEN_PHASE;
unsigned long stateStartTime = 0;
unsigned long currentGreenDuration = BASE_GREEN_TIME;

volatile int northCount = 0;
volatile int southCount = 0;
volatile int eastCount = 0;
volatile int westCount = 0;

#include "state.h"
#include "config.h"

LightState currentState = NS_GREEN_PHASE;
unsigned long stateStartTime = 0;
unsigned long currentGreenDuration = BASE_GREEN_TIME;

int northCount = 0;
int southCount = 0;
int eastCount = 0;
int westCount = 0;

bool lastNorthIn = LOW;
bool lastNorthOut = LOW;
bool lastSouthIn = LOW;
bool lastSouthOut = LOW;
bool lastEastIn = LOW;
bool lastEastOut = LOW;
bool lastWestIn = LOW;
bool lastWestOut = LOW;

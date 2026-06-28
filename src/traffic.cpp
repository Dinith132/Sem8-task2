#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "traffic.h"

unsigned long calcGreenDuration(int density) {
  return BASE_GREEN_TIME + ((unsigned long)density * PER_VEHICLE_TIME);
}

void updateStateMachine() {
  unsigned long elapsed = millis() - stateStartTime;
  int nsDensity = northCount + southCount;
  int ewDensity = eastCount + westCount;

  switch (currentState) {

    case NS_GREEN_PHASE:
      if (elapsed > currentGreenDuration) {
        if (ewDensity >= nsDensity && ewDensity > 0) {
          Serial.println("[STATE] NS_GREEN -> NS_YELLOW (EW will take over)");
          currentState = NS_YELLOW_PHASE;
        } else {
          currentGreenDuration = calcGreenDuration(nsDensity);
          Serial.print("[STATE] NS_GREEN extended, new duration = ");
          Serial.println(currentGreenDuration);
        }
        stateStartTime = millis();
      }
      break;

    case NS_YELLOW_PHASE:
      if (elapsed > YELLOW_TIME) {
        currentGreenDuration = calcGreenDuration(ewDensity);
        Serial.print("[STATE] NS_YELLOW -> EW_GREEN, duration = ");
        Serial.println(currentGreenDuration);
        currentState = EW_GREEN_PHASE;
        stateStartTime = millis();
      }
      break;

    case EW_GREEN_PHASE:
      if (elapsed > currentGreenDuration) {
        if (nsDensity >= ewDensity && nsDensity > 0) {
          Serial.println("[STATE] EW_GREEN -> EW_YELLOW (NS will take over)");
          currentState = EW_YELLOW_PHASE;
        } else {
          currentGreenDuration = calcGreenDuration(ewDensity);
          Serial.print("[STATE] EW_GREEN extended, new duration = ");
          Serial.println(currentGreenDuration);
        }
        stateStartTime = millis();
      }
      break;

    case EW_YELLOW_PHASE:
      if (elapsed > YELLOW_TIME) {
        currentGreenDuration = calcGreenDuration(nsDensity);
        Serial.print("[STATE] EW_YELLOW -> NS_GREEN, duration = ");
        Serial.println(currentGreenDuration);
        currentState = NS_GREEN_PHASE;
        stateStartTime = millis();
      }
      break;
  }
}

void applyLights() {
  bool nsGreen  = (currentState == NS_GREEN_PHASE);
  bool nsYellow = (currentState == NS_YELLOW_PHASE);
  bool nsRed    = (currentState == EW_GREEN_PHASE || currentState == EW_YELLOW_PHASE);

  bool ewGreen  = (currentState == EW_GREEN_PHASE);
  bool ewYellow = (currentState == EW_YELLOW_PHASE);
  bool ewRed    = (currentState == NS_GREEN_PHASE || currentState == NS_YELLOW_PHASE);

  digitalWrite(NS_GREEN, nsGreen);
  digitalWrite(NS_YELLOW, nsYellow);
  digitalWrite(NS_RED, nsRed);
  digitalWrite(EW_GREEN, ewGreen);
  digitalWrite(EW_YELLOW, ewYellow);
  digitalWrite(EW_RED, ewRed);
}

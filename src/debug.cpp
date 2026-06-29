#include <Arduino.h>
#include "state.h"
#include "traffic.h"
#include "debug.h"

void printDebug() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    unsigned long waitA = (millis() - lastGreenTimestamp[LANE_A]) / 1000UL;
    unsigned long waitB = (millis() - lastGreenTimestamp[LANE_B]) / 1000UL;

    Serial.print("[STATUS] N:"); Serial.print(northCount);
    Serial.print(" S:"); Serial.print(southCount);
    Serial.print(" E:"); Serial.print(eastCount);
    Serial.print(" W:"); Serial.print(westCount);
    Serial.print(" | countA:"); Serial.print(northCount + southCount);
    Serial.print(" countB:"); Serial.print(eastCount + westCount);
    Serial.print(" | waitA:"); Serial.print(waitA);
    Serial.print("s waitB:"); Serial.print(waitB);
    Serial.print("s | State:"); Serial.print(currentState);
    Serial.print(" | GreenDur:"); Serial.print(currentGreenDuration);
    Serial.print(" | Last:"); Serial.println(lastDecisionReason);
  }
}

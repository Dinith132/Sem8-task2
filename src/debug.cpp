#include <Arduino.h>
#include "state.h"
#include "debug.h"

void printDebug() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print("[STATUS] N:"); Serial.print(northCount);
    Serial.print(" S:"); Serial.print(southCount);
    Serial.print(" E:"); Serial.print(eastCount);
    Serial.print(" W:"); Serial.print(westCount);
    Serial.print(" | NS_density:"); Serial.print(northCount + southCount);
    Serial.print(" EW_density:"); Serial.print(eastCount + westCount);
    Serial.print(" | State:"); Serial.print(currentState);
    Serial.print(" | GreenDur:"); Serial.println(currentGreenDuration);
  }
}

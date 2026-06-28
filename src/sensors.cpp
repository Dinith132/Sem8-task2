#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "sensors.h"

static void readLaneEdgeTriggered(const char* name, int inPin, bool &lastIn, int outPin, bool &lastOut, int &count) {
  bool currentIn = digitalRead(inPin);
  if (currentIn == HIGH && lastIn == LOW) {
    count++;
    Serial.print("[EVENT] "); Serial.print(name);
    Serial.print(" IN -> count = "); Serial.println(count);
  }
  lastIn = currentIn;

  bool currentOut = digitalRead(outPin);
  if (currentOut == HIGH && lastOut == LOW) {
    count--;
    if (count < 0) count = 0;
    Serial.print("[EVENT] "); Serial.print(name);
    Serial.print(" OUT -> count = "); Serial.println(count);
  }
  lastOut = currentOut;
}

void updateDensity() {
  readLaneEdgeTriggered("NORTH", NORTH_IN, lastNorthIn, NORTH_OUT, lastNorthOut, northCount);
  readLaneEdgeTriggered("SOUTH", SOUTH_IN, lastSouthIn, SOUTH_OUT, lastSouthOut, southCount);
  readLaneEdgeTriggered("EAST",  EAST_IN,  lastEastIn,  EAST_OUT,  lastEastOut,  eastCount);
  readLaneEdgeTriggered("WEST",  WEST_IN,  lastWestIn,  WEST_OUT,  lastWestOut,  westCount);
}

#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "sensors.h"
#include "traffic.h"
#include "display.h"
#include "debug.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== Traffic Junction Boot ===");

  pinMode(NORTH_IN, INPUT_PULLDOWN);
  pinMode(NORTH_OUT, INPUT_PULLDOWN);
  pinMode(SOUTH_IN, INPUT_PULLDOWN);
  pinMode(SOUTH_OUT, INPUT_PULLDOWN);
  pinMode(EAST_IN, INPUT_PULLDOWN);
  pinMode(EAST_OUT, INPUT_PULLDOWN);
  pinMode(WEST_IN, INPUT_PULLDOWN);
  pinMode(WEST_OUT, INPUT_PULLDOWN);

  pinMode(NS_RED, OUTPUT);
  pinMode(NS_YELLOW, OUTPUT);
  pinMode(NS_GREEN, OUTPUT);
  pinMode(EW_RED, OUTPUT);
  pinMode(EW_YELLOW, OUTPUT);
  pinMode(EW_GREEN, OUTPUT);

  displayInit();
  sensorsInit();

  currentGreenDuration = calcGreenDuration(northCount + southCount);
  stateStartTime = millis();
}

void loop() {
  updateStateMachine();
  applyLights();
  updateDisplays();
  printDebug();
}

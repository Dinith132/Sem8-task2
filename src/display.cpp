#include <Arduino.h>
#include <TM1637Display.h>
#include "config.h"
#include "state.h"
#include "display.h"

static TM1637Display nsDisp(CLK, NS_DIO);
static TM1637Display ewDisp(CLK, EW_DIO);

void displayInit() {
  nsDisp.setBrightness(7);
  ewDisp.setBrightness(7);
}

void updateDisplays() {
  unsigned long elapsed = millis() - stateStartTime;
  unsigned long currentPhaseDuration =
      (currentState == NS_YELLOW_PHASE || currentState == EW_YELLOW_PHASE)
        ? YELLOW_TIME : currentGreenDuration;

  long remaining = (long)(currentPhaseDuration - elapsed) / 1000;
  if (remaining < 0) remaining = 0;

  bool nsActive = (currentState == NS_GREEN_PHASE || currentState == NS_YELLOW_PHASE);
  bool ewActive = (currentState == EW_GREEN_PHASE || currentState == EW_YELLOW_PHASE);

  nsDisp.showNumberDec(nsActive ? remaining : 0);
  ewDisp.showNumberDec(ewActive ? remaining : 0);
}

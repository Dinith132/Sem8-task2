#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "sensors.h"

struct SensorChannel {
  int pin;
  volatile int* count;
  bool isOut;
  volatile bool pinHigh;
  volatile unsigned long lastEdgeMs;
  bool lastStableHigh;
};

static SensorChannel northInCh  = { NORTH_IN,  &northCount, false, false, 0, false };
static SensorChannel northOutCh = { NORTH_OUT, &northCount, true,  false, 0, false };
static SensorChannel southInCh  = { SOUTH_IN,  &southCount, false, false, 0, false };
static SensorChannel southOutCh = { SOUTH_OUT, &southCount, true,  false, 0, false };
static SensorChannel eastInCh   = { EAST_IN,   &eastCount,  false, false, 0, false };
static SensorChannel eastOutCh  = { EAST_OUT,  &eastCount,  true,  false, 0, false };
static SensorChannel westInCh   = { WEST_IN,   &westCount,  false, false, 0, false };
static SensorChannel westOutCh  = { WEST_OUT,  &westCount,  true,  false, 0, false };

static SensorChannel* const allChannels[] = {
  &northInCh, &northOutCh, &southInCh, &southOutCh,
  &eastInCh, &eastOutCh, &westInCh, &westOutCh
};

static void IRAM_ATTR sensorIsr(void* arg) {
  auto* ch = static_cast<SensorChannel*>(arg);
  ch->lastEdgeMs = millis();
  ch->pinHigh = digitalRead(ch->pin);
}

static void initChannel(SensorChannel* ch) {
  ch->pinHigh = digitalRead(ch->pin);
  ch->lastStableHigh = ch->pinHigh;
  ch->lastEdgeMs = millis();
  attachInterruptArg(
    digitalPinToInterrupt(ch->pin),
    sensorIsr,
    ch,
    CHANGE
  );
}

static void updateChannel(SensorChannel* ch) {
  if (millis() - ch->lastEdgeMs < SENSOR_DEBOUNCE_MS) {
    return;
  }

  bool stableHigh = ch->pinHigh;

  // Count only on clear -> blocked (stable LOW to stable HIGH).
  if (stableHigh && !ch->lastStableHigh) {
    if (ch->isOut) {
      if (*ch->count > 0) {
        --(*ch->count);
      }
    } else {
      ++(*ch->count);
    }
  }

  ch->lastStableHigh = stableHigh;
}

void sensorsInit() {
  for (SensorChannel* ch : allChannels) {
    initChannel(ch);
  }
}

void sensorsUpdate() {
  for (SensorChannel* ch : allChannels) {
    updateChannel(ch);
  }
}

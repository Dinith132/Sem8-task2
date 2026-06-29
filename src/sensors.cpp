#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "sensors.h"

struct LaneIsrContext {
  volatile int* count;
  bool isOut;
};

static LaneIsrContext northInCtx  = { &northCount, false };
static LaneIsrContext northOutCtx = { &northCount, true };
static LaneIsrContext southInCtx  = { &southCount, false };
static LaneIsrContext southOutCtx = { &southCount, true };
static LaneIsrContext eastInCtx   = { &eastCount, false };
static LaneIsrContext eastOutCtx  = { &eastCount, true };
static LaneIsrContext westInCtx   = { &westCount, false };
static LaneIsrContext westOutCtx  = { &westCount, true };

static void IRAM_ATTR laneIsr(void* arg) {
  auto* ctx = static_cast<LaneIsrContext*>(arg);
  if (ctx->isOut) {
    if (*ctx->count > 0) {
      --(*ctx->count);
    }
  } else {
    ++(*ctx->count);
  }
}

static void attachLaneInterrupt(int pin, LaneIsrContext* ctx) {
  attachInterruptArg(
    digitalPinToInterrupt(pin),
    laneIsr,
    ctx,
    RISING
  );
}

void sensorsInit() {
  attachLaneInterrupt(NORTH_IN,  &northInCtx);
  attachLaneInterrupt(NORTH_OUT, &northOutCtx);
  attachLaneInterrupt(SOUTH_IN,  &southInCtx);
  attachLaneInterrupt(SOUTH_OUT, &southOutCtx);
  attachLaneInterrupt(EAST_IN,   &eastInCtx);
  attachLaneInterrupt(EAST_OUT,  &eastOutCtx);
  attachLaneInterrupt(WEST_IN,   &westInCtx);
  attachLaneInterrupt(WEST_OUT,  &westOutCtx);
}

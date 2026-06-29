#ifndef TRAFFIC_H
#define TRAFFIC_H

#include "state.h"

struct LaneDecision {
  Lane lane;
  const char* reason;
  int scoreA;
  int scoreB;
};

extern const char* lastDecisionReason;

int computeScore(int count, unsigned long waitTimeSec);
unsigned long calcGreenDuration(int count);
LaneDecision decideNextLane(int countA, int countB,
                            unsigned long waitA_sec, unsigned long waitB_sec);
void updateStateMachine();
void applyLights();

#endif

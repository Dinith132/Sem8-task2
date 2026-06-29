#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "traffic.h"

const char* lastDecisionReason = "";

static unsigned long laneWaitSeconds(Lane lane) {
  return (millis() - lastGreenTimestamp[lane]) / 1000UL;
}

static int laneCount(Lane lane) {
  return (lane == LANE_A) ? (northCount + southCount) : (eastCount + westCount);
}

int computeScore(int count, unsigned long waitTimeSec) {
  return count + (int)(SCORE_WAIT_WEIGHT_K * waitTimeSec);
}

unsigned long calcGreenDuration(int count) {
  unsigned long bonus = (unsigned long)count * TIME_PER_VEHICLE_MS;
  unsigned long duration = BASE_GREEN_TIME + bonus;
  return duration > MAX_GREEN_TIME ? MAX_GREEN_TIME : duration;
}

LaneDecision decideNextLane(int countA, int countB,
                            unsigned long waitA_sec, unsigned long waitB_sec) {
  int scoreA = computeScore(countA, waitA_sec);
  int scoreB = computeScore(countB, waitB_sec);

  LaneDecision result;
  result.scoreA = scoreA;
  result.scoreB = scoreB;

  // Layer 2: T_MAX hard override — checked before Layer 1 score comparison
  bool aStarved = waitA_sec >= (unsigned long)T_MAX_SEC;
  bool bStarved = waitB_sec >= (unsigned long)T_MAX_SEC;

  if (aStarved && bStarved) {
    if (waitA_sec >= waitB_sec) {
      result.lane = LANE_A;
      result.reason = "Lane A forced via T_MAX override (both starved, longer wait)";
    } else {
      result.lane = LANE_B;
      result.reason = "Lane B forced via T_MAX override (both starved, longer wait)";
    }
  } else if (aStarved) {
    result.lane = LANE_A;
    result.reason = "Lane A forced via T_MAX override";
  } else if (bStarved) {
    result.lane = LANE_B;
    result.reason = "Lane B forced via T_MAX override";
  } else if (scoreA > scoreB) {
    result.lane = LANE_A;
    result.reason = "Lane A won by score";
  } else if (scoreB > scoreA) {
    result.lane = LANE_B;
    result.reason = "Lane B won by score";
  } else if (waitA_sec >= waitB_sec) {
    result.lane = LANE_A;
    result.reason = "Tie-break: Lane A longer wait";
  } else {
    result.lane = LANE_B;
    result.reason = "Tie-break: Lane B longer wait";
  }

  Serial.print("[DECIDE] countA=");
  Serial.print(countA);
  Serial.print(" countB=");
  Serial.print(countB);
  Serial.print(" | waitA=");
  Serial.print(waitA_sec);
  Serial.print("s waitB=");
  Serial.print(waitB_sec);
  Serial.print("s | scoreA=");
  Serial.print(scoreA);
  Serial.print(" scoreB=");
  Serial.print(scoreB);
  Serial.print(" | -> ");
  Serial.print(result.lane == LANE_A ? "Lane A" : "Lane B");
  Serial.print(" (");
  Serial.print(result.reason);
  Serial.println(")");

  return result;
}

static void enterGreen(Lane lane) {
  int count = laneCount(lane);
  currentGreenDuration = calcGreenDuration(count);
  lastGreenTimestamp[lane] = millis();
  stateStartTime = millis();
  currentState = (lane == LANE_A) ? LANE_A_GREEN : LANE_B_GREEN;

  Serial.print("[STATE] -> ");
  Serial.print(lane == LANE_A ? "LANE_A_GREEN" : "LANE_B_GREEN");
  Serial.print(", duration = ");
  Serial.println(currentGreenDuration);
}

void updateStateMachine() {
  unsigned long elapsed = millis() - stateStartTime;

  switch (currentState) {

    case LANE_A_GREEN:
      if (elapsed > currentGreenDuration) {
        Serial.println("[STATE] LANE_A_GREEN -> LANE_A_YELLOW");
        currentState = LANE_A_YELLOW;
        stateStartTime = millis();
      }
      break;

    case LANE_A_YELLOW:
      if (elapsed > YELLOW_TIME) {
        Serial.println("[STATE] LANE_A_YELLOW -> DECIDE");
        currentState = DECIDE_PHASE;
        stateStartTime = millis();
      }
      break;

    case LANE_B_GREEN:
      if (elapsed > currentGreenDuration) {
        Serial.println("[STATE] LANE_B_GREEN -> LANE_B_YELLOW");
        currentState = LANE_B_YELLOW;
        stateStartTime = millis();
      }
      break;

    case LANE_B_YELLOW:
      if (elapsed > YELLOW_TIME) {
        Serial.println("[STATE] LANE_B_YELLOW -> DECIDE");
        currentState = DECIDE_PHASE;
        stateStartTime = millis();
      }
      break;

    case DECIDE_PHASE: {
      int countA = northCount + southCount;
      int countB = eastCount + westCount;
      unsigned long waitA = laneWaitSeconds(LANE_A);
      unsigned long waitB = laneWaitSeconds(LANE_B);
      LaneDecision decision = decideNextLane(countA, countB, waitA, waitB);
      lastDecisionReason = decision.reason;
      enterGreen(decision.lane);
      break;
    }
  }
}

void applyLights() {
  bool nsGreen  = (currentState == LANE_A_GREEN);
  bool nsYellow = (currentState == LANE_A_YELLOW);
  bool nsRed    = (currentState == LANE_B_GREEN || currentState == LANE_B_YELLOW
                   || currentState == DECIDE_PHASE);

  bool ewGreen  = (currentState == LANE_B_GREEN);
  bool ewYellow = (currentState == LANE_B_YELLOW);
  bool ewRed    = (currentState == LANE_A_GREEN || currentState == LANE_A_YELLOW
                   || currentState == DECIDE_PHASE);

  digitalWrite(NS_GREEN, nsGreen);
  digitalWrite(NS_YELLOW, nsYellow);
  digitalWrite(NS_RED, nsRed);
  digitalWrite(EW_GREEN, ewGreen);
  digitalWrite(EW_YELLOW, ewYellow);
  digitalWrite(EW_RED, ewRed);
}

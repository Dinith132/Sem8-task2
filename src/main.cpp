#include <Arduino.h>
#include <TM1637Display.h>

// ===== SENSOR PINS =====
const int NORTH_IN  = 32;
const int NORTH_OUT = 33;
const int SOUTH_IN  = 18;
const int SOUTH_OUT = 19;
const int EAST_IN   = 25;
const int EAST_OUT  = 26;
const int WEST_IN   = 21;
const int WEST_OUT  = 23;

// ===== LED PINS =====
const int NS_RED    = 13;
const int NS_YELLOW = 5;
const int NS_GREEN  = 4;
const int EW_RED    = 17;
const int EW_YELLOW = 16;
const int EW_GREEN  = 14;

// ===== DISPLAY PINS =====
#define CLK 2
const int NS_DIO = 22;
const int EW_DIO = 27;
TM1637Display nsDisp(CLK, NS_DIO);
TM1637Display ewDisp(CLK, EW_DIO);

// ===== DENSITY COUNTERS =====
int northCount = 0, southCount = 0, eastCount = 0, westCount = 0;

// ===== STATE MACHINE =====
enum LightState { NS_GREEN_PHASE, NS_YELLOW_PHASE, EW_GREEN_PHASE, EW_YELLOW_PHASE };
LightState currentState = NS_GREEN_PHASE;
unsigned long stateStartTime = 0;

const unsigned long BASE_GREEN_TIME  = 2000; // ms
const unsigned long PER_VEHICLE_TIME = 0; // ms per vehicle
const unsigned long YELLOW_TIME      = 2000; // ms fixed

unsigned long currentGreenDuration = BASE_GREEN_TIME;

// edge-detection memory
bool lastNorthIn = LOW, lastNorthOut = LOW;
bool lastSouthIn = LOW, lastSouthOut = LOW;
bool lastEastIn  = LOW, lastEastOut  = LOW;
bool lastWestIn  = LOW, lastWestOut  = LOW;

void updateDensity();
void updateStateMachine();
void applyLights();
void updateDisplays();
void printDebug();
void readLaneEdgeTriggered(const char* name, int inPin, bool &lastIn, int outPin, bool &lastOut, int &count);
unsigned long calcGreenDuration(int density);

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

  pinMode(NS_RED, OUTPUT);   pinMode(NS_YELLOW, OUTPUT);  pinMode(NS_GREEN, OUTPUT);
  pinMode(EW_RED, OUTPUT);   pinMode(EW_YELLOW, OUTPUT);  pinMode(EW_GREEN, OUTPUT);

  nsDisp.setBrightness(7);
  ewDisp.setBrightness(7);

  currentGreenDuration = calcGreenDuration(northCount + southCount);
  stateStartTime = millis();
}

void loop() {
  updateDensity();
  updateStateMachine();
  applyLights();
  updateDisplays();
  printDebug();
}

// ---------- SENSOR READING ----------
void updateDensity() {
  readLaneEdgeTriggered("NORTH", NORTH_IN, lastNorthIn, NORTH_OUT, lastNorthOut, northCount);
  readLaneEdgeTriggered("SOUTH", SOUTH_IN, lastSouthIn, SOUTH_OUT, lastSouthOut, southCount);
  readLaneEdgeTriggered("EAST",  EAST_IN,  lastEastIn,  EAST_OUT,  lastEastOut,  eastCount);
  readLaneEdgeTriggered("WEST",  WEST_IN,  lastWestIn,  WEST_OUT,  lastWestOut,  westCount);
}

void readLaneEdgeTriggered(const char* name, int inPin, bool &lastIn, int outPin, bool &lastOut, int &count) {
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

// ---------- GREEN TIME FORMULA ----------
unsigned long calcGreenDuration(int density) {
  return BASE_GREEN_TIME + ((unsigned long)density * PER_VEHICLE_TIME);
}

// ---------- STATE MACHINE ----------
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

// ---------- LED OUTPUT ----------
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

// ---------- DISPLAY OUTPUT ----------
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

// ---------- DEBUG ----------
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
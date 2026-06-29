#ifndef CONFIG_H
#define CONFIG_H

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

// ===== ALGORITHM TUNABLES (seconds — tune here) =====
#define SCORE_WAIT_WEIGHT_K     0.5f    // K in score = count + K * wait_sec
#define T_MAX_SEC               45
#define BASE_GREEN_TIME_SEC     10
#define MAX_GREEN_TIME_SEC      20
#define TIME_PER_VEHICLE_SEC    0.5f    // small per-vehicle green bonus

// Derived millisecond values for millis()-based timing
const unsigned long T_MAX_MS            = (unsigned long)T_MAX_SEC * 1000UL;
const unsigned long BASE_GREEN_TIME     = (unsigned long)BASE_GREEN_TIME_SEC * 1000UL;
const unsigned long MAX_GREEN_TIME      = (unsigned long)MAX_GREEN_TIME_SEC * 1000UL;
const unsigned long TIME_PER_VEHICLE_MS = (unsigned long)(TIME_PER_VEHICLE_SEC * 1000.0f);
const unsigned long YELLOW_TIME         = 2000; // ms fixed

#endif

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

// ===== TIMING =====
const unsigned long BASE_GREEN_TIME  = 2000; // ms
const unsigned long PER_VEHICLE_TIME = 0;   // ms per vehicle
const unsigned long YELLOW_TIME      = 2000; // ms fixed

#endif

#pragma once

static const int MAX_AIRCRAFT = 30;

struct Aircraft {
  char  callsign[9];   // flight number, e.g. EZY1234
  char  typeCode[5];   // ICAO type, e.g. A20N
  char  reg[8];        // registration, e.g. G-EZWA
  float lat;
  float lon;
  float altitude;      // feet
  bool  valid;
};

extern Aircraft      aircraft[MAX_AIRCRAFT];
extern int           aircraftCount;
extern bool          lastFetchOk;
extern int           lastHttpCode;
extern int           totalFetches;
extern volatile bool fetchInProgress;
extern volatile bool fetchComplete;

void fetchAircraft();

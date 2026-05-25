#pragma once

struct Config {
  float lat;
  float lon;
  float range_km;
};

void          configLoad();
void          configSave(float lat, float lon, float range_km);
const Config& configGet();

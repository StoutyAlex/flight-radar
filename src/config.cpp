#include "config.h"
#include <Preferences.h>

static Config _cfg = { 53.39f, -2.64f, 20.0f };

void configLoad() {
  Preferences p;
  p.begin("radar", true);
  _cfg.lat      = p.getFloat("lat",   53.39f);
  _cfg.lon      = p.getFloat("lon",   -2.64f);
  _cfg.range_km = p.getFloat("range", 20.0f);
  p.end();
}

void configSave(float lat, float lon, float range_km) {
  Preferences p;
  p.begin("radar", false);
  p.putFloat("lat",   lat);
  p.putFloat("lon",   lon);
  p.putFloat("range", range_km);
  p.end();
  _cfg.lat      = lat;
  _cfg.lon      = lon;
  _cfg.range_km = range_km;
}

const Config& configGet() { return _cfg; }

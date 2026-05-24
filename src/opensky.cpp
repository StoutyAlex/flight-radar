
#include "opensky.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

// 20km ≈ 10.8nm, round to 11nm for the airplanes.live radius parameter
static const char* URL =
  "https://api.airplanes.live/v2/point/53.39/-2.64/11";

Aircraft         aircraft[MAX_AIRCRAFT];
int              aircraftCount   = 0;
bool             lastFetchOk     = false;
int              lastHttpCode    = 0;
int              totalFetches    = 0;
volatile bool    fetchInProgress = false;
volatile bool    fetchComplete   = false;

static void trimSpaces(char* s) {
  int len = strlen(s);
  while (len > 0 && s[len - 1] == ' ') s[--len] = '\0';
}

void fetchAircraft() {
  Serial.printf("[fetch] starting (WiFi: %s)\n",
                WiFi.status() == WL_CONNECTED ? "up" : "down");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(8000);
  if (!http.begin(client, URL)) {
    Serial.println("[fetch] http.begin failed");
    return;
  }

  lastHttpCode = http.GET();
  Serial.printf("[fetch] HTTP %d\n", lastHttpCode);
  if (lastHttpCode != HTTP_CODE_OK) {
    http.end();
    lastFetchOk = false;
    return;
  }

  String body = http.getString();
  http.end();
  Serial.printf("[fetch] body: %d bytes\n", body.length());

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.printf("[fetch] JSON error: %s\n", err.c_str());
    lastFetchOk = false;
    return;
  }

  JsonArray ac = doc["ac"].as<JsonArray>();
  Aircraft temp[MAX_AIRCRAFT];
  int count = 0;

  for (JsonObject a : ac) {
    if (count >= MAX_AIRCRAFT) break;

    // Skip aircraft on the ground (alt_baro field missing or tagged "ground")
    JsonVariant altVar = a["alt_baro"];
    if (altVar.isNull() || altVar.is<const char*>()) continue;
    float alt = altVar.as<float>();
    if (alt < 100.0f) continue;

    JsonVariant lat = a["lat"];
    JsonVariant lon = a["lon"];
    if (lat.isNull() || lon.isNull()) continue;

    Aircraft& t = temp[count];

    // Callsign / flight number
    const char* fl = a["flight"].as<const char*>();
    if (fl && fl[0] != '\0') {
      strncpy(t.callsign, fl, 8); t.callsign[8] = '\0';
      trimSpaces(t.callsign);
    } else {
      const char* hex = a["hex"].as<const char*>();
      strncpy(t.callsign, hex ? hex : "??????", 8); t.callsign[8] = '\0';
    }

    // ICAO type code
    const char* type = a["t"].as<const char*>();
    strncpy(t.typeCode, type ? type : "????", 4); t.typeCode[4] = '\0';

    // Registration
    const char* reg = a["r"].as<const char*>();
    strncpy(t.reg, reg ? reg : "???????", 7); t.reg[7] = '\0';

    t.lat      = lat.as<float>();
    t.lon      = lon.as<float>();
    t.altitude = alt;
    t.valid    = true;
    count++;
  }

  // Fast swap — display blocked only for this memcpy
  fetchInProgress = true;
  memcpy(aircraft, temp, sizeof(Aircraft) * count);
  for (int i = count; i < aircraftCount; i++) aircraft[i].valid = false;
  aircraftCount   = count;
  fetchInProgress = false;

  lastFetchOk   = true;
  fetchComplete = true;
  totalFetches++;
}

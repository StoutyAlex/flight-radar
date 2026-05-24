#include "wifi_manager.h"
#include <WiFi.h>

static const char* SSID     = "Apollo House";
static const char* PASSWORD = "aabbcc1122";

void wifiBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  int n = WiFi.scanNetworks();
  bool found = false;
  for (int i = 0; i < n; i++) {
    if (strcmp(WiFi.SSID(i).c_str(), SSID) == 0) {
      Serial.printf("[wifi] found '%s' %d dBm ch%d\n", SSID, WiFi.RSSI(i), WiFi.channel(i));
      found = true;
    }
  }
  if (!found) Serial.printf("[wifi] WARNING: '%s' not found in scan\n", SSID);

  Serial.printf("[wifi] connecting to '%s'\n", SSID);
  WiFi.begin(SSID, PASSWORD);
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void wifiMaintain() {
  wl_status_t status = (wl_status_t)WiFi.status();

  // Log status every 5s while not connected
  static uint32_t lastLog = 0;
  uint32_t now = millis();
  if (status != WL_CONNECTED && now - lastLog >= 5000) {
    lastLog = now;
    Serial.printf("[wifi] status=%d\n", (int)status);
  }

  if (status == WL_CONNECTED) {
    static uint32_t lastRssiLog = 0;
    if (now - lastRssiLog >= 10000) {
      lastRssiLog = now;
      Serial.printf("[wifi] connected, RSSI=%d dBm\n", WiFi.RSSI());
    }
    return;
  }

  // Safety net: if the driver's auto-reconnect has stalled, force a fresh attempt
  static uint32_t lastAttempt = 0;
  if (now - lastAttempt < 60000) return;
  lastAttempt = now;

  Serial.println("[wifi] forcing reconnect");
  WiFi.disconnect(false);
  delay(500);
  WiFi.begin(SSID, PASSWORD);
}

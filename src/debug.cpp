#include "display.h"
#include "debug.h"
#include "wifi_manager.h"
#include "opensky.h"
#include <WiFi.h>
#include <time.h>

void drawDebug(uint32_t lastFetchMs) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  char buf[32];

  // Postcode header
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("WA5 3UE", 120, 30, 1);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("-- DEBUG --", 120, 42, 1);

  // Time (NTP)
  struct tm t;
  if (getLocalTime(&t, 0)) {
    strftime(buf, sizeof(buf), "%H:%M:%S", &t);
  } else {
    strncpy(buf, "--:--:--", sizeof(buf));
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(buf, 120, 58, 1);

  // WiFi status + RSSI
  if (wifiConnected()) {
    int rssi = WiFi.RSSI();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("WiFi: CONNECTED", 120, 76, 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(WiFi.localIP().toString().c_str(), 120, 88, 1);
    // Colour-code RSSI: green > -70, yellow > -80, red <= -80
    uint16_t rssiColour = (rssi > -70) ? TFT_GREEN : (rssi > -80) ? TFT_YELLOW : TFT_RED;
    tft.setTextColor(rssiColour, TFT_BLACK);
    snprintf(buf, sizeof(buf), "Signal: %d dBm", rssi);
    tft.drawString(buf, 120, 100, 1);
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("WiFi: CONNECTING...", 120, 76, 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Signal: --", 120, 100, 1);
  }

  // Fetch status
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (lastHttpCode == 0) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Fetch: pending", 120, 118, 1);
  } else if (lastFetchOk) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Fetch: OK", 120, 118, 1);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    snprintf(buf, sizeof(buf), "Fetch: FAIL (%d)", lastHttpCode);
    tft.drawString(buf, 120, 118, 1);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  snprintf(buf, sizeof(buf), "Aircraft: %d", aircraftCount);
  tft.drawString(buf, 120, 136, 1);

  snprintf(buf, sizeof(buf), "Fetches: %d", totalFetches);
  tft.drawString(buf, 120, 150, 1);

  // Time since last fetch
  if (lastFetchMs == 0) {
    tft.drawString("Last fetch: --", 120, 164, 1);
  } else {
    uint32_t ago = (millis() - lastFetchMs) / 1000;
    snprintf(buf, sizeof(buf), "Last: %lus ago", ago);
    tft.drawString(buf, 120, 164, 1);
  }
}

#include "display.h"
#include "radar.h"
#include "debug.h"
#include "wifi_manager.h"
#include "opensky.h"
#include "fetch_task.h"
#include <time.h>

#define BUTTON_PIN 5  // GPIO5

static const uint32_t FETCH_INTERVAL_MS  = 20000;
static const uint32_t WIFI_WAKE_AHEAD_MS = 5000;
static const uint32_t DEBUG_REFRESH_MS   = 1000;

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

bool     debugMode        = false;
bool     lastButtonState  = HIGH;
uint32_t lastFetchTime    = 0;
uint32_t lastDebugRefresh = 0;

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.init();
  tft.setRotation(0);

  spr.setColorDepth(8);
  spr.createSprite(240, 240);

  drawRadarBackground();
  wifiBegin();
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);
  tzset();
  initFetchTask();
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    debugMode = !debugMode;
    if (debugMode) {
      drawDebug(lastFetchTime);
      lastDebugRefresh = millis();
    } else {
      drawRadarBackground();
      resetSweep();
    }
    delay(200);
  }
  lastButtonState = buttonState;

  if (debugMode) {
    uint32_t now = millis();
    if (now - lastDebugRefresh >= DEBUG_REFRESH_MS) {
      lastDebugRefresh = now;
      drawDebug(lastFetchTime);
    }
  } else {
    updateSweep();
  }

  wifiMaintain();

  uint32_t now = millis();

  // Wake WiFi ahead of the next scheduled fetch
  if (wifiAsleep() && lastFetchTime > 0) {
    if (now - lastFetchTime >= FETCH_INTERVAL_MS - WIFI_WAKE_AHEAD_MS) {
      wifiWake();
    }
  }

  if (wifiConnected()) {
    if (lastFetchTime == 0 || now - lastFetchTime >= FETCH_INTERVAL_MS) {
      lastFetchTime = now;
      requestFetch();
    }
  }

  if (fetchComplete) {
    fetchComplete = false;
    wifiSleep();
  }

  delay(1);
}

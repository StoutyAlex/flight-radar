#include "display.h"
#include "radar.h"
#include "debug.h"
#include "wifi_manager.h"
#include "opensky.h"
#include "fetch_task.h"
#include "config.h"
#include "setup_server.h"
#include <time.h>

#define BUTTON_PIN 5  // GPIO5

static const uint32_t FETCH_INTERVAL_MS    = 20000;
static const uint32_t WIFI_WAKE_AHEAD_MS   = 5000;
static const uint32_t DEBUG_REFRESH_MS     = 1000;
static const uint32_t LONG_PRESS_MS        = 2000;

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

bool     debugMode        = false;
bool     setupMode        = false;
bool     lastButtonState  = HIGH;
uint32_t lastFetchTime    = 0;
uint32_t lastDebugRefresh = 0;
uint32_t buttonPressTime  = 0;
bool     longPressArmed   = false;

static void enterSetupMode() {
  setupMode = true;
  debugMode = false;
  wifiSleep();
  wifiBegin();
  setupServerBegin();
}

static void exitSetupMode() {
  setupServerStop();
  setupMode    = false;
  lastFetchTime = 0;
  wifiSleep();
  tft.fillScreen(TFT_BLACK);
  spr.fillSprite(TFT_BLACK);
  drawRadarBackground();
  resetSweep();
  wifiWake();
}

void setup() {
  Serial.begin(115200);

  configLoad();

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

  // Button press tracking
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressTime = millis();
    longPressArmed  = true;
  }

  if (buttonState == HIGH && lastButtonState == LOW) {
    // Short press released
    if (longPressArmed) {
      if (!setupMode) {
        debugMode = !debugMode;
        if (debugMode) {
          drawDebug(lastFetchTime);
          lastDebugRefresh = millis();
        } else {
          drawRadarBackground();
          resetSweep();
        }
      }
      delay(200);
    }
    longPressArmed = false;
  }

  // Long press: enter or exit setup mode
  if (longPressArmed && buttonState == LOW) {
    if (millis() - buttonPressTime >= LONG_PRESS_MS) {
      longPressArmed = false;
      if (setupMode) {
        exitSetupMode();
      } else {
        enterSetupMode();
      }
      delay(200);
    }
  }

  lastButtonState = buttonState;

  if (setupMode) {
    setupServerHandle();
    delay(1);
    return;
  }

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

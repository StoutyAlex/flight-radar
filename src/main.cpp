#include "display.h"
#include "radar.h"
#include "debug.h"
#include "wifi_manager.h"
#include "opensky.h"
#include "fetch_task.h"
#include <time.h>
#include <esp_system.h>

#define BUTTON_PIN 5  // D2 = GPIO5

static const uint32_t FETCH_INTERVAL_MS = 15000;
static const uint32_t DEBUG_REFRESH_MS  = 1000;

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

bool     debugMode        = false;
bool     lastButtonState  = HIGH;
uint32_t lastFetchTime    = 0;
uint32_t lastDebugRefresh = 0;

static const char* resetReasonStr(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "power-on";
    case ESP_RST_EXT:      return "external pin";
    case ESP_RST_SW:       return "software";
    case ESP_RST_PANIC:    return "PANIC/crash";
    case ESP_RST_INT_WDT:  return "interrupt watchdog";
    case ESP_RST_TASK_WDT: return "task watchdog";
    case ESP_RST_WDT:      return "watchdog";
    case ESP_RST_DEEPSLEEP:return "deep sleep wakeup";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    default:               return "unknown";
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("[boot] reset reason: %s\n", resetReasonStr(reason));

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.init();
  tft.setRotation(0);

  spr.setColorDepth(8);
  bool sprOk = spr.createSprite(240, 240);
  Serial.printf("[boot] sprite %s  heap=%u\n", sprOk ? "ok" : "FAILED", ESP.getFreeHeap());

  drawRadarBackground();
  wifiBegin();
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);
  tzset();
  initFetchTask();
  Serial.println("[boot] ready");
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    debugMode = !debugMode;
    if (debugMode) {
      drawDebug(lastFetchTime);
      lastDebugRefresh = millis();
      Serial.println("Debug mode");
    } else {
      drawRadarBackground();
      resetSweep();
      Serial.println("Radar mode");
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

  if (wifiConnected()) {
    uint32_t now = millis();
    if (lastFetchTime == 0 || now - lastFetchTime >= FETCH_INTERVAL_MS) {
      lastFetchTime = now;
      requestFetch();
    }
  }

  // Fetch complete — aircraft data is updated; sweep picks it up automatically
  if (fetchComplete) {
    fetchComplete = false;
  }

  delay(1);
}

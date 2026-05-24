#include "display.h"
#include "radar.h"
#include "debug.h"

#define BUTTON_PIN 5  // D2 = GPIO5

TFT_eSPI tft = TFT_eSPI();

bool debugMode = false;
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.init();
  tft.setRotation(0);
  drawRadarBackground();
  Serial.println("Ready");
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    debugMode = !debugMode;
    if (debugMode) {
      drawDebug();
      Serial.println("Debug mode");
    } else {
      drawRadarBackground();
      resetSweep();
      Serial.println("Radar mode");
    }
    delay(50);
  }
  lastButtonState = buttonState;

  if (!debugMode) updateSweep();

  delay(1);
}

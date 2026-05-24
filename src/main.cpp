#include <TFT_eSPI.h>
#include <math.h>

#define BUTTON_PIN 5  // D2 = GPIO5

TFT_eSPI tft = TFT_eSPI();

bool debugMode = false;
bool lastButtonState = HIGH;

const int CX = 120;
const int CY = 120;
const int R  = 115;

const float    SWEEP_STEP = 2.0f;
const uint32_t FRAME_MS   = 20;
const int      TRAIL_LEN  = 12;

float    sweepAngle    = 0;
uint32_t lastSweepTime = 0;

const uint16_t TRAIL_COLORS[] = {
  0x07E0, 0x0640, 0x04C0, 0x0380,
  0x0280, 0x01C0, 0x0140, 0x00E0,
  0x00A0, 0x0060, 0x0040, 0x0020,
  0x0000
};

void sweepEnd(float deg, int &x2, int &y2) {
  float rad = deg * M_PI / 180.0f;
  x2 = CX + (int)(R * sinf(rad) + 0.5f);
  y2 = CY - (int)(R * cosf(rad) + 0.5f);
}

void drawRingsAndCrosshairs() {
  for (int i = 1; i <= 4; i++) {
    tft.drawCircle(CX, CY, R * i / 4, TFT_DARKGREEN);
  }
  tft.drawLine(CX, CY - R, CX, CY + R, TFT_DARKGREEN);
  tft.drawLine(CX - R, CY, CX + R, CY, TFT_DARKGREEN);
}

void drawLabels() {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(117,   4); tft.print("N");
  tft.setCursor(117, 228); tft.print("S");
  tft.setCursor(228, 116); tft.print("E");
  tft.setCursor(  4, 116); tft.print("W");
  tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.setCursor(CX + R/4 + 2,   CY - 8); tft.print("25");
  tft.setCursor(CX + R/2 + 2,   CY - 8); tft.print("50");
  tft.setCursor(CX + 3*R/4 + 2, CY - 8); tft.print("75");
}

void drawRadarBackground() {
  tft.fillScreen(TFT_BLACK);
  drawRingsAndCrosshairs();
  drawLabels();
}

void updateSweep() {
  uint32_t now = millis();
  if (now - lastSweepTime < FRAME_MS) return;
  lastSweepTime = now;

  int x2, y2;

  // Erase previous sweep position
  sweepEnd(sweepAngle - SWEEP_STEP, x2, y2);
  tft.drawLine(CX, CY, x2, y2, TFT_BLACK);

  // Draw new sweep line
  sweepEnd(sweepAngle, x2, y2);
  tft.drawLine(CX, CY, x2, y2, TFT_GREEN);

  // Labels first, then crosshairs on top — so text background can't erase them
  drawLabels();
  drawRingsAndCrosshairs();

  sweepAngle = fmodf(sweepAngle + SWEEP_STEP, 360.0f);
}

void drawDebug() {
  tft.fillScreen(TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextSize(2);
  tft.setCursor(65, 110);
  tft.print("DEBUG");
}

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
      sweepAngle = 0;
      Serial.println("Radar mode");
    }
    delay(50);
  }
  lastButtonState = buttonState;

  if (!debugMode) updateSweep();

  delay(1);
}

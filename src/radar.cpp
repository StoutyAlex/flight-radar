#include "display.h"
#include "radar.h"
#include <math.h>

static const int CX = 120;
static const int CY = 120;
static const int R  = 115;

static const float    SWEEP_STEP = 2.0f;
static const uint32_t FRAME_MS   = 20;

static float    sweepAngle    = 0;
static uint32_t lastSweepTime = 0;

static void sweepEnd(float deg, int &x2, int &y2) {
  float rad = deg * M_PI / 180.0f;
  x2 = CX + (int)(R * sinf(rad) + 0.5f);
  y2 = CY - (int)(R * cosf(rad) + 0.5f);
}

static void drawRingsAndCrosshairs() {
  for (int i = 1; i <= 4; i++) {
    tft.drawCircle(CX, CY, R * i / 4, TFT_DARKGREEN);
  }
  tft.drawLine(CX, CY - R, CX, CY + R, TFT_DARKGREEN);
  tft.drawLine(CX - R, CY, CX + R, CY, TFT_DARKGREEN);
}

static void drawLabels() {
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

  sweepEnd(sweepAngle - SWEEP_STEP, x2, y2);
  tft.drawLine(CX, CY, x2, y2, TFT_BLACK);

  sweepEnd(sweepAngle, x2, y2);
  tft.drawLine(CX, CY, x2, y2, TFT_GREEN);

  // Labels before crosshairs so text background can't erase ring pixels
  drawLabels();
  drawRingsAndCrosshairs();

  sweepAngle = fmodf(sweepAngle + SWEEP_STEP, 360.0f);
}

void resetSweep() {
  sweepAngle = 0;
}

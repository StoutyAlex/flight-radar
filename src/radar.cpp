#include "display.h"
#include "radar.h"
#include "opensky.h"
#include <math.h>

static const int CX = 120;
static const int CY = 120;
static const int R  = 115;

static const float    SWEEP_STEP = 2.0f;
static const uint32_t FRAME_MS   = 20;

// WA5 3UE centre point, 20km range
static const float CENTER_LAT     = 53.39f;
static const float CENTER_LON     = -2.64f;
static const float KM_PER_DEG_LAT = 111.0f;
static const float KM_PER_DEG_LON = 66.02f;  // 111 * cos(53.39°)
static const float RANGE_KM       = 20.0f;

static float    sweepAngle    = 0;
static uint32_t lastSweepTime = 0;

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
  tft.setCursor(CX + R/4 + 2,   CY - 8); tft.print("5");
  tft.setCursor(CX + R/2 + 2,   CY - 8); tft.print("10");
  tft.setCursor(CX + 3*R/4 + 2, CY - 8); tft.print("15");
}

void drawRadarBackground() {
  tft.fillScreen(TFT_BLACK);
  drawRingsAndCrosshairs();
  drawLabels();
}

void plotAircraft() {
  if (fetchInProgress) return;

  // Snapshot count once — fetch task may update aircraftCount on Core 0
  int count = aircraftCount;
  for (int i = 0; i < count; i++) {
    const Aircraft& a = aircraft[i];
    if (!a.valid) continue;

    float dy_km = (a.lat - CENTER_LAT) * KM_PER_DEG_LAT;
    float dx_km = (a.lon - CENTER_LON) * KM_PER_DEG_LON;

    if (sqrtf(dx_km * dx_km + dy_km * dy_km) > RANGE_KM) continue;

    int px = CX + (int)(dx_km / RANGE_KM * R);
    int py = CY - (int)(dy_km / RANGE_KM * R);

    tft.fillCircle(px, py, 2, TFT_GREEN);

    tft.setTextSize(1);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(px + 4, py - 4);
    tft.print(a.callsign);

    tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    tft.setCursor(px + 4, py + 4);
    tft.print(a.typeCode);
    tft.print(" ");
    tft.print(a.reg);
  }
}

void updateSweep() {
  uint32_t now = millis();
  if (now - lastSweepTime < FRAME_MS) return;
  lastSweepTime = now;

  int x2, y2;
  float rad;

  rad = (sweepAngle - SWEEP_STEP) * M_PI / 180.0f;
  x2  = CX + (int)(R * sinf(rad) + 0.5f);
  y2  = CY - (int)(R * cosf(rad) + 0.5f);
  tft.drawLine(CX, CY, x2, y2, TFT_BLACK);

  rad = sweepAngle * M_PI / 180.0f;
  x2  = CX + (int)(R * sinf(rad) + 0.5f);
  y2  = CY - (int)(R * cosf(rad) + 0.5f);
  tft.drawLine(CX, CY, x2, y2, TFT_GREEN);

  // Labels before crosshairs so text background can't erase ring pixels
  drawLabels();
  drawRingsAndCrosshairs();
  plotAircraft();

  sweepAngle = fmodf(sweepAngle + SWEEP_STEP, 360.0f);
}

void resetSweep() {
  sweepAngle = 0;
}

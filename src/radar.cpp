#include "display.h"
#include "radar.h"
#include "opensky.h"
#include "routes.h"
#include <math.h>

static const int CX = 120;
static const int CY = 120;
static const int R  = 115;

static const float    SWEEP_STEP = 2.0f;
static const uint32_t FRAME_MS   = 20;

static const float CENTER_LAT     = 53.39f;
static const float CENTER_LON     = -2.64f;
static const float KM_PER_DEG_LAT = 111.0f;
static const float KM_PER_DEG_LON = 66.02f;
static const float RANGE_KM       = 20.0f;

static float    sweepAngle    = 0;
static uint32_t lastSweepTime = 0;

static void drawRingsAndCrosshairs() {
    for (int i = 1; i <= 4; i++)
        spr.drawCircle(CX, CY, R * i / 4, TFT_DARKGREEN);
    spr.drawLine(CX, CY - R, CX, CY + R, TFT_DARKGREEN);
    spr.drawLine(CX - R, CY, CX + R, CY, TFT_DARKGREEN);
}

static void drawLabels() {
    spr.setTextSize(1);
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.setCursor(117,   4); spr.print("N");
    spr.setCursor(117, 228); spr.print("S");
    spr.setCursor(228, 116); spr.print("E");
    spr.setCursor(  4, 116); spr.print("W");
    spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    spr.setCursor(CX + R/4 + 2,   CY - 8); spr.print("5");
    spr.setCursor(CX + R/2 + 2,   CY - 8); spr.print("10");
    spr.setCursor(CX + 3*R/4 + 2, CY - 8); spr.print("15");
}

// Filled triangle pointing in heading direction (0=North, clockwise)
static void drawPlaneIcon(int px, int py, float heading, uint16_t color) {
    float rad = heading * M_PI / 180.0f;
    float s = sinf(rad);
    float c = cosf(rad);

    // Nose: 5px ahead
    int x0 = px + (int)(s * 5);
    int y0 = py - (int)(c * 5);
    // Left base: 3px back, 4px left
    int x1 = px - (int)(s * 3) - (int)(c * 4);
    int y1 = py + (int)(c * 3) - (int)(s * 4);
    // Right base: 3px back, 4px right
    int x2 = px - (int)(s * 3) + (int)(c * 4);
    int y2 = py + (int)(c * 3) + (int)(s * 4);

    spr.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

static void plotAircraft() {
    if (fetchInProgress) return;

    int count = aircraftCount;
    for (int i = 0; i < count; i++) {
        const Aircraft& a = aircraft[i];
        if (!a.valid) continue;

        float dy_km = (a.lat - CENTER_LAT) * KM_PER_DEG_LAT;
        float dx_km = (a.lon - CENTER_LON) * KM_PER_DEG_LON;
        if (sqrtf(dx_km * dx_km + dy_km * dy_km) > RANGE_KM) continue;

        int px = CX + (int)(dx_km / RANGE_KM * R);
        int py = CY - (int)(dy_km / RANGE_KM * R);

        if (a.heading >= 0.0f) {
            drawPlaneIcon(px, py, a.heading, TFT_GREEN);
        } else {
            spr.fillCircle(px, py, 2, TFT_GREEN);
        }

        spr.setTextSize(1);
        spr.setTextColor(TFT_GREEN, TFT_BLACK);
        spr.setCursor(px + 7, py - 4);
        spr.print(a.callsign);

        spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
        spr.setCursor(px + 7, py + 4);
        spr.print(a.typeCode);
        spr.print(" ");
        spr.print(a.reg);

        if (!routeCacheUpdating) {
            const RouteEntry* route = findRoute(a.callsign);
            if (route && route->fetched && route->origin[0] != '\0') {
                spr.setTextColor(TFT_CYAN, TFT_BLACK);
                spr.setCursor(px + 7, py + 12);
                spr.print(route->origin);
                spr.print(">");
                spr.print(route->dest);
            }
        }
    }
}

void drawRadarBackground() {
    spr.fillScreen(TFT_BLACK);
    drawRingsAndCrosshairs();
    drawLabels();
    spr.pushSprite(0, 0);
}

void updateSweep() {
    uint32_t now = millis();
    if (now - lastSweepTime < FRAME_MS) return;
    lastSweepTime = now;

    // Full redraw to sprite each frame — eliminates flicker entirely
    spr.fillScreen(TFT_BLACK);
    drawRingsAndCrosshairs();
    drawLabels();

    // Sweep line
    float rad = sweepAngle * M_PI / 180.0f;
    int x2 = CX + (int)(R * sinf(rad) + 0.5f);
    int y2 = CY - (int)(R * cosf(rad) + 0.5f);
    spr.drawLine(CX, CY, x2, y2, TFT_GREEN);

    plotAircraft();
    spr.pushSprite(0, 0);

    sweepAngle = fmodf(sweepAngle + SWEEP_STEP, 360.0f);
}

void resetSweep() {
    sweepAngle = 0;
}

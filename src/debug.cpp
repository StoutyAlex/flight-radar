#include "display.h"
#include "debug.h"

void drawDebug() {
  tft.fillScreen(TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextSize(2);
  tft.setCursor(65, 110);
  tft.print("DEBUG");
}

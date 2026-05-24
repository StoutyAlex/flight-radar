#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void printChipInfo() {
  Serial.println("\n=== Chip Info ===");
  Serial.printf("  CPU freq:  %d MHz\n",  ESP.getCpuFreqMHz());
  Serial.printf("  Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("  SDK:       %s\n",       ESP.getSdkVersion());
}

void printPinConfig() {
  Serial.println("=== TFT Pin Config (compiled) ===");
  Serial.printf("  MOSI: %d\n",  TFT_MOSI);
  Serial.printf("  SCLK: %d\n",  TFT_SCLK);
  Serial.printf("  CS:   %d\n",  TFT_CS);
  Serial.printf("  DC:   %d\n",  TFT_DC);
  Serial.printf("  RST:  %d\n",  TFT_RST);
  Serial.printf("  SPI:  %d MHz\n", SPI_FREQUENCY / 1000000);
#ifdef USE_HSPI_PORT
  Serial.println("  Bus:  HSPI");
#else
  Serial.println("  Bus:  VSPI/default");
#endif
}

void colorTest() {
  Serial.println("=== Color Test ===");
  struct { uint16_t bg; uint16_t fg; const char* name; } tests[] = {
    { TFT_RED,   TFT_WHITE, "RED"   },
    { TFT_GREEN, TFT_BLACK, "GREEN" },
    { TFT_BLUE,  TFT_WHITE, "BLUE"  },
    { TFT_WHITE, TFT_BLACK, "WHITE" },
  };
  for (auto& t : tests) {
    tft.fillScreen(t.bg);
    tft.setTextColor(t.fg, t.bg);
    tft.setTextSize(2);
    tft.setCursor(80, 115);
    tft.print(t.name);
    Serial.printf("  %s\n", t.name);
    delay(800);
  }
}

void drawTestPattern() {
  Serial.println("=== Test Pattern ===");
  tft.fillScreen(TFT_BLACK);
  tft.drawLine(120, 0,   120, 240, TFT_DARKGREY);
  tft.drawLine(0,   120, 240, 120, TFT_DARKGREY);
  tft.drawCircle(120, 120, 119, TFT_WHITE);
  tft.fillRect(0,   0,   8, 8, TFT_RED);
  tft.fillRect(232, 0,   8, 8, TFT_GREEN);
  tft.fillRect(232, 232, 8, 8, TFT_BLUE);
  tft.fillRect(0,   232, 8, 8, TFT_YELLOW);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== Boot ===");

  printChipInfo();
  printPinConfig();

  Serial.print("Calling tft.init()... ");
  tft.init();
  tft.setRotation(0);
  Serial.println("done");
  Serial.printf("  Display: %d x %d\n", tft.width(), tft.height());

  colorTest();
  drawTestPattern();
  delay(1500);

  tft.fillScreen(TFT_BLACK);
  tft.fillCircle(120, 120, 100, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(55, 115);
  tft.println("Hello World!");

  Serial.println("Setup complete.");
}

void loop() {}

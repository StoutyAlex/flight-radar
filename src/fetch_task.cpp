#include "fetch_task.h"
#include "opensky.h"
#include "routes.h"
#include "wifi_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t trigger;

static void fetchLoop(void*) {
  while (true) {
    xSemaphoreTake(trigger, portMAX_DELAY);
    if (wifiConnected()) {
      fetchAircraft();
      pruneRouteCache();
      lookupPendingRoute();
    }
  }
}

void initFetchTask() {
  trigger = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(fetchLoop, "fetch", 32768, nullptr, 1, nullptr, 0);
}

void requestFetch() {
  xSemaphoreGive(trigger);
}

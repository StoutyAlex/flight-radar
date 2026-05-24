#include "routes.h"
#include "opensky.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#include <ctype.h>

RouteEntry    routeCache[MAX_ROUTES];
volatile bool routeCacheUpdating = false;
static int    cacheCount = 0;

static bool isHexCallsign(const char* s) {
    int len = strlen(s);
    if (len != 6) return false;
    for (int i = 0; i < 6; i++) {
        char c = tolower((unsigned char)s[i]);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    }
    return true;
}

const RouteEntry* findRoute(const char* callsign) {
    for (int i = 0; i < cacheCount; i++) {
        if (routeCache[i].valid && strcmp(routeCache[i].callsign, callsign) == 0)
            return &routeCache[i];
    }
    return nullptr;
}

void pruneRouteCache() {
    routeCacheUpdating = true;
    for (int i = 0; i < cacheCount; i++) {
        if (!routeCache[i].valid) continue;
        bool found = false;
        for (int j = 0; j < aircraftCount; j++) {
            if (aircraft[j].valid && strcmp(aircraft[j].callsign, routeCache[i].callsign) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            Serial.printf("[route] evict %s\n", routeCache[i].callsign);
            routeCache[i] = routeCache[--cacheCount];
            routeCache[cacheCount].valid = false;
            i--;
        }
    }
    routeCacheUpdating = false;
}

static bool fetchRoute(const char* callsign, char* origin, char* dest) {
    char url[64];
    snprintf(url, sizeof(url), "https://api.adsbdb.com/v0/callsign/%s", callsign);

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setTimeout(5000);
    if (!http.begin(client, url)) return false;

    int code = http.GET();
    if (code != 200) { http.end(); return false; }

    String body = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, body)) return false;

    JsonVariant fr = doc["response"]["flightroute"];
    if (fr.isNull()) return false;

    const char* org = fr["origin"]["iata_code"].as<const char*>();
    const char* dst = fr["destination"]["iata_code"].as<const char*>();
    if (!org || !dst || org[0] == '\0' || dst[0] == '\0') return false;

    strncpy(origin, org, 4); origin[4] = '\0';
    strncpy(dest,   dst, 4); dest[4]   = '\0';
    return true;
}

void lookupPendingRoute() {
    for (int i = 0; i < aircraftCount; i++) {
        const Aircraft& a = aircraft[i];
        if (!a.valid) continue;
        if (findRoute(a.callsign)) continue;
        if (cacheCount >= MAX_ROUTES) break;

        // Reserve slot immediately so it isn't queued twice
        routeCacheUpdating = true;
        RouteEntry& e = routeCache[cacheCount++];
        strncpy(e.callsign, a.callsign, 8); e.callsign[8] = '\0';
        e.origin[0] = '\0'; e.dest[0] = '\0';
        e.valid   = true;
        e.fetched = isHexCallsign(a.callsign);  // hex codes need no lookup
        routeCacheUpdating = false;

        if (e.fetched) continue;  // was a hex code, move on

        // One HTTP lookup per cycle
        char origin[5] = {}, dest[5] = {};
        bool ok = fetchRoute(a.callsign, origin, dest);

        routeCacheUpdating = true;
        if (ok) {
            strncpy(e.origin, origin, 4); e.origin[4] = '\0';
            strncpy(e.dest,   dest,   4); e.dest[4]   = '\0';
            Serial.printf("[route] %s: %s>%s\n", e.callsign, e.origin, e.dest);
        } else {
            Serial.printf("[route] %s: no route\n", e.callsign);
        }
        e.fetched = true;
        routeCacheUpdating = false;

        return;  // one HTTP lookup per fetch cycle
    }
}

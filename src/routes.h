#pragma once

static const int MAX_ROUTES = 30;

struct RouteEntry {
    char callsign[9];
    char origin[5];
    char dest[5];
    bool valid;
    bool fetched;
};

extern RouteEntry    routeCache[MAX_ROUTES];
extern volatile bool routeCacheUpdating;

const RouteEntry* findRoute(const char* callsign);
void pruneRouteCache();
void lookupPendingRoute();

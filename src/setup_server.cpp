#include "setup_server.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <qrcode.h>
#include "config.h"
#include "display.h"

static WebServer server(80);

static const char HTML_TMPL[] =
  "<!DOCTYPE html><html><head>"
  "<meta charset='utf-8'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>Radar Setup</title>"
  "<style>"
  "body{font-family:sans-serif;max-width:420px;margin:0 auto;padding:16px;background:#111;color:#eee}"
  "h1{margin:0 0 16px;font-size:1.3em}"
  "label{display:block;margin:14px 0 4px;font-size:.9em;color:#aaa}"
  "input{width:100%%;box-sizing:border-box;padding:8px;background:#222;border:1px solid #444;color:#eee;font-size:1em;border-radius:4px}"
  ".row{display:flex;gap:8px}.row input{flex:1}"
  "input[type=range]{padding:4px 0;background:none;border:none}"
  "button{padding:10px 14px;border-radius:4px;border:none;cursor:pointer;font-size:.95em;margin-top:6px}"
  ".geo,.pc-btn{background:#444;color:#eee;width:100%%}"
  ".pc-row{display:flex;gap:8px;margin-top:4px}.pc-row input{flex:1}"
  ".pc-btn{width:auto}"
  ".save{background:#0074d9;color:#fff;width:100%%;margin-top:16px;font-size:1.1em}"
  "#st{font-size:.8em;color:#888;margin:6px 0 0;min-height:1.2em}"
  "</style></head><body>"
  "<h1>&#x1F4E1; Radar Setup</h1>"
  "<form method='POST' action='/save'>"
  "<label>Range: <strong id='rv'>%dkm</strong></label>"
  "<input type='range' name='range_km' id='r' min='10' max='150' step='5' value='%d'"
  " oninput=\"document.getElementById('rv').textContent=this.value+'km'\">"
  "<label>Centre location</label>"
  "<div class='row'>"
  "<input type='number' name='lat' id='lat' placeholder='Latitude' step='any' value='%.4f' required>"
  "<input type='number' name='lon' id='lon' placeholder='Longitude' step='any' value='%.4f' required>"
  "</div>"
  "<button type='button' class='geo' onclick='geo()'>&#x1F4CD; Use my location</button>"
  "<label>Or enter UK postcode</label>"
  "<div class='pc-row'>"
  "<input type='text' id='pc' placeholder='e.g. SK11 0AB'>"
  "<button type='button' class='pc-btn' onclick='lookup()'>Look up</button>"
  "</div>"
  "<p id='st'></p>"
  "<button type='submit' class='save'>Save &amp; Restart</button>"
  "</form>"
  "<script>"
  "function geo(){"
  "var s=document.getElementById('st');"
  "s.textContent='Getting location\xe2\x80\xa6';"
  "navigator.geolocation.getCurrentPosition("
  "function(p){"
  "document.getElementById('lat').value=p.coords.latitude.toFixed(4);"
  "document.getElementById('lon').value=p.coords.longitude.toFixed(4);"
  "s.textContent='Location set \xe2\x9c\x93';},"
  "function(e){s.textContent='Error: '+e.message;});"
  "}"
  "function lookup(){"
  "var pc=document.getElementById('pc').value.trim().replace(/\\s/g,'');"
  "if(!pc)return;"
  "var s=document.getElementById('st');"
  "s.textContent='Looking up\xe2\x80\xa6';"
  "fetch('https://api.postcodes.io/postcodes/'+pc)"
  ".then(r=>r.json())"
  ".then(d=>{"
  "if(d.status===200){"
  "document.getElementById('lat').value=d.result.latitude.toFixed(4);"
  "document.getElementById('lon').value=d.result.longitude.toFixed(4);"
  "s.textContent='Found \xe2\x9c\x93';}"
  "else{s.textContent='Not found';}})"
  ".catch(()=>s.textContent='Lookup failed');}"
  "</script></body></html>";

static void drawSetupScreen() {
  const char* url = "http://radar.local";

  QRCode qrcode;
  uint8_t buf[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, buf, 3, ECC_LOW, url);

  // White background directly on TFT (no sprite — setup screen is static)
  tft.fillScreen(TFT_WHITE);

  // QR code: version 3 = 29 modules, scale 5 → 145px, fully centred.
  // Corner-to-centre distance ≈ 103px, safely inside the 120px circle radius.
  const int scale = 5;
  const int qsize = qrcode.size * scale;
  const int ox    = (240 - qsize) / 2;
  const int oy    = (240 - qsize) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      uint16_t col = qrcode_getModule(&qrcode, x, y) ? TFT_BLACK : TFT_WHITE;
      tft.fillRect(ox + x * scale, oy + y * scale, scale, scale, col);
    }
  }
}

static void handleRoot() {
  const Config& c = configGet();
  static char html[2800];
  snprintf(html, sizeof(html), HTML_TMPL,
           (int)c.range_km, (int)c.range_km, c.lat, c.lon);
  server.send(200, "text/html", html);
}

static void handleSave() {
  float lat      = server.arg("lat").toFloat();
  float lon      = server.arg("lon").toFloat();
  float range_km = server.arg("range_km").toFloat();

  if (lat < -90 || lat > 90 || lon < -180 || lon > 180 ||
      range_km < 10 || range_km > 150) {
    server.send(400, "text/plain", "Invalid values");
    return;
  }

  configSave(lat, lon, range_km);
  server.send(200, "text/html",
    "<html><body style='font-family:sans-serif;background:#111;color:#eee;padding:32px'>"
    "<h2>Saved! Restarting radar...</h2></body></html>");
  delay(600);
  ESP.restart();
}

void setupServerBegin() {
  MDNS.begin("radar");
  server.on("/",     HTTP_GET,  handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  drawSetupScreen();
}

void setupServerStop() {
  server.stop();
  MDNS.end();
}

void setupServerHandle() {
  server.handleClient();
}

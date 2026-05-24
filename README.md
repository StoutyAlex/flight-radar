# Flight Radar

A real-time aircraft radar display built on an Arduino Nano ESP32, showing live ADS-B traffic within 20 km of WA5 3UE (Warrington, UK) on a circular TFT screen.

## Features

- Animated radar sweep with range rings (5 / 10 / 15 km)
- Live aircraft positions, callsigns, ICAO type codes and registrations
- Data from [airplanes.live](https://airplanes.live) — free, no API key required
- Background FreeRTOS fetch task so the display never lags during HTTP requests
- Button toggle between radar view and debug screen
- Debug screen: NTP time, WiFi status, RSSI, fetch stats, aircraft count

## Components

| Component | Details |
|---|---|
| **Microcontroller** | Arduino Nano ESP32 (ESP32-S3, 240 MHz, 16 MB Flash) |
| **Display** | 1.28" Round GC9A01 IPS LCD, 240×240 px |
| **Button** | Tactile push button (radar ↔ debug toggle) |

## Wiring / Pin Layout

All SPI pins are on the Arduino Nano ESP32. The Nano uses raw GPIO numbers (not `D` labels) in firmware.

### Display (GC9A01 → Nano ESP32)

| Display Pin | Nano Pin | GPIO |
|---|---|---|
| VCC | 3.3V | — |
| GND | GND | — |
| SCL (SCLK) | D12 | GPIO 47 |
| SDA (MOSI) | D11 | GPIO 38 |
| CS | D10 | GPIO 21 |
| DC | D9 | GPIO 18 |
| RST | D8 | GPIO 17 |
| BL | 3.3V | — |

### Button

| Connection | Nano Pin | GPIO |
|---|---|---|
| One leg | D2 | GPIO 5 |
| Other leg | GND | — |

The pin is configured `INPUT_PULLUP` — no external resistor needed.

## Software

- **Framework:** Arduino (via PlatformIO)
- **Platform:** `espressif32`
- **Board:** `arduino_nano_esp32`
- **Upload protocol:** DFU (double-tap reset button → solid green LED)

### Libraries

| Library | Version |
|---|---|
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | ^2.5.43 |
| [ArduinoJson](https://arduinojson.org) | ^7 |

### TFT_eSPI config

Copy `User_Setups/Setup200_GC9A01.h` into the TFT_eSPI library folder and set `User_Setup_Select.h` to include it, or let PlatformIO pick it up via the existing `build_flags` in `platformio.ini`.

## Flashing

1. Double-tap the reset button — the onboard LED should glow solid green (DFU mode)
2. `pio run --target upload`
3. Press reset once to boot the new firmware

## Data source

Aircraft data is fetched every 15 seconds from:

```
https://api.airplanes.live/v2/point/53.39/-2.64/11
```

(`53.39°N, 2.64°W` = WA5 3UE centre point, `11` = 11 nm radius ≈ 20 km)

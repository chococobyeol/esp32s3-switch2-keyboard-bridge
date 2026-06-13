#pragma once

// Copy to config.local.h and fill in local-only settings.
// Do not commit config.local.h.

// Current stable/default mode: ESP32 joins your home Wi-Fi.
#define NK_WIFI_MODE NK_WIFI_MODE_HOME
#define NK_WIFI_SSID "YOUR_WIFI_SSID"
#define NK_WIFI_PASS "YOUR_WIFI_PASSWORD"

// Optional direct AP mode. This keeps ESP32 independent from a router,
// but your laptop/phone must join this AP and may lose Internet on Wi-Fi.
// #define NK_WIFI_MODE NK_WIFI_MODE_AP
// #define NK_AP_SSID "NSGamepad"
// #define NK_AP_PASS ""

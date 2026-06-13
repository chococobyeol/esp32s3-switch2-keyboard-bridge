#pragma once

// Public, non-secret defaults. Put private credentials in config.local.h.
// config.local.h is ignored by git and should not be committed.
#if __has_include("config.local.h")
#include "config.local.h"
#endif

#define NK_WIFI_MODE_HOME 1
#define NK_WIFI_MODE_AP 2

#ifndef NK_WIFI_MODE
#define NK_WIFI_MODE NK_WIFI_MODE_HOME
#endif

#ifndef NK_WIFI_SSID
#define NK_WIFI_SSID ""
#endif

#ifndef NK_WIFI_PASS
#define NK_WIFI_PASS ""
#endif

#ifndef NK_AP_SSID
#define NK_AP_SSID "NSGamepad"
#endif

// Leave empty for open AP. Use 8+ chars if setting a password.
#ifndef NK_AP_PASS
#define NK_AP_PASS ""
#endif

#ifndef NK_WIFI_CONNECT_ATTEMPTS
#define NK_WIFI_CONNECT_ATTEMPTS 40
#endif

#ifndef NK_WIFI_CONNECT_DELAY_MS
#define NK_WIFI_CONNECT_DELAY_MS 500
#endif

// Generic ESP32-S3 DevKit mode. Disable M5Stack AtomS3 display/LED/button code.
#ifndef NK_DISABLE_M5_UI
#define NK_DISABLE_M5_UI 1
#endif

// Keep disabled on this board: periodic HID reports destabilized HTTP/WebSocket.
// Browser repeats active inputs instead.
#ifndef NK_DISABLE_GAMEPAD_LOOP
#define NK_DISABLE_GAMEPAD_LOOP 1
#endif

#ifndef NK_VERBOSE_WS
#define NK_VERBOSE_WS 0
#endif

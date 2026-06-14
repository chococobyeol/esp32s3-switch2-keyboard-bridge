/*
 * ESP32-S3 WebSocket keyboard/gamepad bridge for Nintendo Switch-compatible USB HID
 */

/*
 * MIT License
 *
 * Copyright (c) 2023 touchgadgetdev@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"

#ifndef DEBUG_ON
#define DEBUG_ON  1
#endif
#define DIAG_DISABLE_GAMEPAD_LOOP NK_DISABLE_GAMEPAD_LOOP
#define DIAG_VERBOSE_WS NK_VERBOSE_WS
#define DIAG_NO_M5_UI NK_DISABLE_M5_UI
#if DEBUG_ON
#define DBG_begin(...)    Serial.begin(__VA_ARGS__)
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#define DBG_printf(...)   Serial.printf(__VA_ARGS__)
#else
#define DBG_begin(...)
#define DBG_print(...)
#define DBG_println(...)
#define DBG_printf(...)
#endif

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>  // See README.md
#include <ArduinoJson.h>  // Install from IDE Library manager
#include <WebSocketsServer.h> // Install WebSockets by Markus Sattler from IDE Library manager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneButton.h>  // https://github.com/mathertel/OneButton
#include <FastLED.h>    // https://github.com/FastLED/FastLED
#include <M5GFX.h>      // https://github.com/m5stack/M5GFX
#include "index_html.h"
#include "switch_ESP32.h"

/* WiFi Network */
MDNSResponder mdns;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
#include "gamepad_grid.h"

/* USB gamepad */
NSGamepad Gamepad;

/* M5Stack AtomS3 display, RGB LED, button */
M5GFX display;

static const int LED_DI_PIN = 35;
CRGB RGBled;

static const int BTN_PIN = 41;
OneButton button(BTN_PIN, true);

static const int TEXT_SIZE = 3;

void clrscr() {
  display.clear(TFT_BLACK);
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'display.println'
  //  or stay on the line is there is room for the text with display.print)
  display.setCursor(0, 0);
  display.setTextSize(TEXT_SIZE);
}

void setup_display() {
  display.begin();

  if (display.isEPD())
  {
    display.setEpdMode(epd_mode_t::epd_fastest);
    display.invertDisplay(true);
    display.clear(TFT_BLACK);
  }
  if (display.width() < display.height())
  {
    display.setRotation(display.getRotation() ^ 1);
  }

  display.setColorDepth(8);
  display.setTextWrap(true);
  display.setTextSize(TEXT_SIZE);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  clrscr(); display.print("USBNSWA");
}

/* USB Gamepad */
static bool dpad_up, dpad_down, dpad_left, dpad_right;
static String latestUiStateJson;
static String activeUiOwner;
static uint32_t latestControlStateSeq = 0;
static String activeControlOwner;

static uint8_t clamp_axis_value(const int value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return (uint8_t)value;
}

void control_down_value(const int control, const int value);
void release_all_controls();

void control_down(const int control) {
  if (control <= NSButton_Reserved2) {
    Gamepad.press(control);
  }
  else switch (control) {
    case NSDPad_Up:
      dpad_up = true;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Down:
      dpad_down = true;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Left:
      dpad_left = true;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Right:
      dpad_right = true;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;

    case NSLeftStick_Up_Lock:
      /* fall through */
    case NSLeftStick_Up:
      Gamepad.leftYAxis(0);
      break;
    case NSLeftStick_Up_Lock_Walk:
      /* fall through */
    case NSLeftStick_Up_Walk:
      Gamepad.leftYAxis(64);
      break;
    case NSLeftStick_Down:
      Gamepad.leftYAxis(255);
      break;
    case NSLeftStick_Left:
      Gamepad.leftXAxis(0);
      break;
    case NSLeftStick_Right:
      Gamepad.leftXAxis(255);
      break;
    case NSRightStick_Up:
      Gamepad.rightYAxis(0);
      break;
    case NSRightStick_Down:
      Gamepad.rightYAxis(255);
      break;
    case NSRightStick_Left:
      Gamepad.rightXAxis(0);
      break;
    case NSRightStick_Right:
      Gamepad.rightXAxis(255);
      break;
    default:
      break;
  }
}

void control_down_value(const int control, const int value) {
  if (value < 0) {
    control_down(control);
    return;
  }
  const uint8_t axis = clamp_axis_value(value);
  switch (control) {
    case NSLeftStick_Up_Lock:
    case NSLeftStick_Up_Lock_Walk:
    case NSLeftStick_Up_Walk:
    case NSLeftStick_Up:
    case NSLeftStick_Down:
      Gamepad.leftYAxis(axis);
      break;
    case NSLeftStick_Left:
    case NSLeftStick_Right:
      Gamepad.leftXAxis(axis);
      break;
    case NSRightStick_Up:
    case NSRightStick_Down:
      Gamepad.rightYAxis(axis);
      break;
    case NSRightStick_Left:
    case NSRightStick_Right:
      Gamepad.rightXAxis(axis);
      break;
    default:
      control_down(control);
      break;
  }
}

void control_up(const int control) {
  if (control <= NSButton_Reserved2) {
    Gamepad.release(control);
  }
  else switch (control) {
    case NSDPad_Up:
      dpad_up = false;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Down:
      dpad_down = false;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Left:
      dpad_left = false;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;
    case NSDPad_Right:
      dpad_right = false;
      Gamepad.dPad(dpad_up, dpad_down, dpad_left, dpad_right);
      break;

    case NSLeftStick_Up:
      /* fall through */
    case NSLeftStick_Up_Walk:
      /* fall through */
    case NSLeftStick_Down:
      Gamepad.leftYAxis(128);
      break;
    case NSLeftStick_Left:
      /* fall through */
    case NSLeftStick_Right:
      Gamepad.leftXAxis(128);
      break;
    case NSRightStick_Up:
      /* fall through */
    case NSRightStick_Down:
      Gamepad.rightYAxis(128);
      break;
    case NSRightStick_Left:
      /* fall through */
    case NSRightStick_Right:
      Gamepad.rightXAxis(128);
      break;
    default:
      break;
  }
}

void release_all_controls() {
  for (int button = 0; button <= NSButton_Reserved2; button++) {
    Gamepad.release(button);
  }
  dpad_up = false;
  dpad_down = false;
  dpad_left = false;
  dpad_right = false;
  Gamepad.dPad(false, false, false, false);
  Gamepad.leftXAxis(128);
  Gamepad.leftYAxis(128);
  Gamepad.rightXAxis(128);
  Gamepad.rightYAxis(128);
}

/* WiFi Network */

/* WiFiManager, global */
WiFiManager wm;

void send_latest_state_to(const uint8_t client) {
  if (latestUiStateJson.length() > 0) {
    webSocket.sendTXT(client, latestUiStateJson);
  }
}

String payload_to_string(const uint8_t *payload, const size_t length) {
  String message;
  message.reserve(length + 1);
  for (size_t i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  return message;
}

static const size_t MAX_RELAY_STATE_BYTES = 4096;

static const char* ALLOWED_KEY_CODES[] = {
  "Escape", "Digit1", "Digit2", "Digit3", "Digit4", "Digit5", "Digit6", "Digit7", "Digit8", "Digit9", "Digit0", "Minus", "Equal", "Backspace",
  "Tab", "KeyQ", "KeyW", "KeyE", "KeyR", "KeyT", "KeyY", "KeyU", "KeyI", "KeyO", "KeyP", "BracketLeft", "BracketRight", "Backslash",
  "CapsLock", "KeyA", "KeyS", "KeyD", "KeyF", "KeyG", "KeyH", "KeyJ", "KeyK", "KeyL", "Semicolon", "Quote", "Enter",
  "ShiftLeft", "KeyZ", "KeyX", "KeyC", "KeyV", "KeyB", "KeyN", "KeyM", "Comma", "Period", "Slash", "ShiftRight",
  "ControlLeft", "MetaLeft", "AltLeft", "Space", "AltRight", "MetaRight", "ArrowLeft", "ArrowUp", "ArrowDown", "ArrowRight"
};

static const char* ALLOWED_CONTROL_IDS[] = {
  "Y", "B", "A", "X", "L", "R", "ZL", "ZR", "Minus", "Plus", "LSB", "RSB", "Home", "Capture",
  "DPadUp", "DPadDown", "DPadLeft", "DPadRight",
  "LeftStickUp", "LeftStickDown", "LeftStickLeft", "LeftStickRight",
  "RightStickUp", "RightStickDown", "RightStickLeft", "RightStickRight"
};

bool string_in_list(const char* value, const char* const* list, const size_t count) {
  if (!value || !value[0]) return false;
  for (size_t i = 0; i < count; i++) {
    if (strcmp(value, list[i]) == 0) return true;
  }
  return false;
}

bool is_allowed_key_code(const char* value) {
  return string_in_list(value, ALLOWED_KEY_CODES, sizeof(ALLOWED_KEY_CODES) / sizeof(ALLOWED_KEY_CODES[0]));
}

bool is_allowed_control_id(const char* value) {
  return string_in_list(value, ALLOWED_CONTROL_IDS, sizeof(ALLOWED_CONTROL_IDS) / sizeof(ALLOWED_CONTROL_IDS[0]));
}

bool is_allowed_mode(const char* value) {
  return strcmp(value, "full") == 0 || strcmp(value, "half") == 0;
}

bool validate_keymap(JsonVariantConst keymap) {
  if (keymap.isNull()) return true;
  if (!keymap.is<JsonObjectConst>()) return false;
  JsonObjectConst obj = keymap.as<JsonObjectConst>();
  for (JsonPairConst kv : obj) {
    const char* key = kv.key().c_str();
    const char* control = kv.value() | "";
    if (!is_allowed_key_code(key) || !is_allowed_control_id(control)) return false;
  }
  return true;
}

bool validate_settings(JsonVariantConst settings) {
  if (settings.isNull()) return true;
  if (!settings.is<JsonObjectConst>()) return false;
  JsonObjectConst obj = settings.as<JsonObjectConst>();
  if (obj["halfPercent"].is<int>()) {
    const int half = obj["halfPercent"].as<int>();
    if (half < 5 || half > 95) return false;
  }
  if (!obj["leftToggleKey"].isNull() && !is_allowed_key_code(obj["leftToggleKey"] | "")) return false;
  if (!obj["rightToggleKey"].isNull() && !is_allowed_key_code(obj["rightToggleKey"] | "")) return false;
  return true;
}

bool validate_modes(JsonVariantConst modes) {
  if (modes.isNull()) return true;
  if (!modes.is<JsonObjectConst>()) return false;
  JsonObjectConst obj = modes.as<JsonObjectConst>();
  if (!obj["left"].isNull() && !is_allowed_mode(obj["left"] | "")) return false;
  if (!obj["right"].isNull() && !is_allowed_mode(obj["right"] | "")) return false;
  return true;
}

bool validate_pressed(JsonVariantConst pressed) {
  if (pressed.isNull()) return true;
  if (!pressed.is<JsonObjectConst>()) return false;
  JsonObjectConst obj = pressed.as<JsonObjectConst>();
  for (JsonPairConst kv : obj) {
    const char* id = kv.key().c_str();
    if (strncmp(id, "key:", 4) != 0) return false;
    if (!is_allowed_key_code(id + 4)) return false;
    if (!kv.value().is<bool>()) return false;
  }
  return true;
}

bool is_valid_owner(const char* owner) {
  if (!owner) return false;
  const size_t len = strlen(owner);
  if (len < 4 || len > 64) return false;
  for (size_t i = 0; i < len; i++) {
    const char ch = owner[i];
    const bool ok = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_';
    if (!ok) return false;
  }
  return true;
}

bool validate_ui_state_message(JsonDocument &doc, String &ownerOut) {
  const int version = doc["v"] | 0;
  const char* type = doc["type"] | "";
  if (version != 1) return false;
  if (strcmp(type, "state_snapshot") != 0 && strcmp(type, "state_patch") != 0) return false;
  const char* owner = doc["owner"] | "";
  if (!is_valid_owner(owner)) return false;
  ownerOut = owner;
  if (strcmp(type, "state_patch") == 0 && activeUiOwner.length() > 0 && activeUiOwner != ownerOut) return false;
  return validate_keymap(doc["keymap"]) &&
         validate_settings(doc["settings"]) &&
         validate_modes(doc["modes"]) &&
         validate_pressed(doc["pressed"]);
}

void relay_state_message(const String &message) {
  latestUiStateJson = message;
  webSocket.broadcastTXT(message.c_str(), message.length());
}

void handle_versioned_message(const uint8_t client, JsonDocument &doc) {
  const int version = doc["v"] | 0;
  if (version != 1) return;
  const char* type = doc["type"] | "";
  if (strcmp(type, "input") == 0) {
    const char* action = doc["action"] | "";
    const int control = doc["control"] | -1;
    const int value = doc["value"] | -1;
    const uint32_t seq = doc["seq"] | 0;
    if (control < 0) return;
    const uint32_t recvMs = millis();
    if (strcmp(action, "up") == 0) {
      control_up(control);
    } else {
      control_down_value(control, value);
    }
    Gamepad.write();
    StaticJsonDocument<160> ack;
    ack["v"] = 1;
    ack["type"] = "input_ack";
    ack["seq"] = seq;
    ack["control"] = control;
    ack["action"] = action;
    ack["recvMs"] = recvMs;
    ack["writeMs"] = millis();
    String json;
    serializeJson(ack, json);
    webSocket.sendTXT(client, json);
  }
  else if (strcmp(type, "control_state") == 0) {
    const uint32_t seq = doc["seq"] | 0;
    const bool wantsAck = doc["ack"] | false;
    const char* reason = doc["reason"] | "state";
    const char* owner = doc["owner"] | "";
    JsonArrayConst controls = doc["controls"].as<JsonArrayConst>();
    if (controls.isNull()) return;
    if (is_valid_owner(owner) && activeControlOwner != owner) {
      activeControlOwner = owner;
      latestControlStateSeq = 0;
      release_all_controls();
      Gamepad.write();
    }

    if (seq != 0 && latestControlStateSeq != 0 && seq < latestControlStateSeq) {
      if (wantsAck) {
        StaticJsonDocument<160> ack;
        ack["v"] = 1;
        ack["type"] = "input_ack";
        ack["seq"] = seq;
        ack["control"] = -1;
        ack["action"] = "stale";
        ack["recvMs"] = millis();
        ack["writeMs"] = millis();
        String json;
        serializeJson(ack, json);
        webSocket.sendTXT(client, json);
      }
      return;
    }
    latestControlStateSeq = seq;

    const uint32_t recvMs = millis();
    release_all_controls();
    size_t applied = 0;
    for (JsonObjectConst item : controls) {
      if (applied++ >= 28) break;
      const int control = item["control"] | -1;
      const int value = item["value"] | -1;
      if (control < 0 || control > NSRightStick_Right) continue;
      control_down_value(control, value);
    }
    Gamepad.write();

    if (wantsAck) {
      StaticJsonDocument<160> ack;
      ack["v"] = 1;
      ack["type"] = "input_ack";
      ack["seq"] = seq;
      ack["control"] = -1;
      ack["action"] = reason;
      ack["recvMs"] = recvMs;
      ack["writeMs"] = millis();
      String json;
      serializeJson(ack, json);
      webSocket.sendTXT(client, json);
    }
  }
  else if (strcmp(type, "state_request") == 0) {
    send_latest_state_to(client);
  }
  else if (strcmp(type, "ping") == 0) {
    StaticJsonDocument<96> reply;
    reply["v"] = 1;
    reply["type"] = "pong";
    reply["t"] = doc["t"] | 0.0;
    String json;
    serializeJson(reply, json);
    webSocket.sendTXT(client, json);
  }
}

void handle_legacy_message(JsonDocument &doc) {
  const char* event = doc["event"] | "";
  if (strcmp(event, "keyup") == 0) {
    const int control = doc["code"] | -1;
    if (control >= 0) control_up(control);
  }
  else if (strcmp(event, "keydown") == 0) {
    const int control = doc["code"] | -1;
    if (control >= 0) control_down(control);
  }
  else {
    int row = doc["row"] | 0;
    if (row < 0) {
      DBG_printf("row negative %d\n", row);
      row = 0;
    }
    if (row >= MAX_ROWS) {
      DBG_printf("row too high %d\n", row);
      row = MAX_ROWS - 1;
    }

    int col = doc["col"] | 0;
    if (col < 0) {
      DBG_printf("col negative %d\n", col);
      col = 0;
    }
    if (col >= MAX_COLS) {
      DBG_printf("col too high %d\n", col);
      col = MAX_COLS - 1;
    }

    const int control = Gamepad_Cells[row][col].gamepad_control;

    if (strcmp(event, "touch start") == 0) {
      control_down(control);
    }
    else if (strcmp(event, "touch end") == 0 || strcmp(event, "touch cancel") == 0) {
      control_up(control);
    }
  }
  Gamepad.write();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
#if DIAG_VERBOSE_WS
      DBG_printf("[%u] Disconnected!\r\n", num);
#endif
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
#if DIAG_VERBOSE_WS
        DBG_printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
#endif
        send_latest_state_to(num);
      }
      break;
    case WStype_TEXT:
      {
#if DIAG_VERBOSE_WS
        DBG_printf("[%u] get Text: [%d] %s \r\n", num, length, payload);
#endif
        if (length > MAX_RELAY_STATE_BYTES) {
          DBG_println(F("WebSocket JSON message too large"));
          return;
        }

        String message = payload_to_string(payload, length);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, message);

        if (error) {
          DBG_print(F("deserializeJson() failed: "));
          DBG_println(error.f_str());
          return;
        }

        const char* messageType = doc["type"] | "";
        if (strcmp(messageType, "state_snapshot") == 0 || strcmp(messageType, "state_patch") == 0) {
          String owner;
          if (validate_ui_state_message(doc, owner)) {
            if (strcmp(messageType, "state_snapshot") == 0) activeUiOwner = owner;
            relay_state_message(message);
          }
          else {
            DBG_println(F("Rejected invalid UI state message"));
          }
          return;
        }

        if (doc.containsKey("type")) {
          handle_versioned_message(num, doc);
        }
        else {
          handle_legacy_message(doc);
        }
      }
      break;
    case WStype_BIN:
#if DIAG_VERBOSE_WS
      DBG_printf("[%u] get binary length: %u\r\n", num, length);
#endif
      break;
    default:
#if DIAG_VERBOSE_WS
      DBG_printf("Invalid WStype [%d]\r\n", type);
#endif
      break;
  }
}

void handleRoot()
{
  server.send_P(200, PSTR("text/html"), INDEX_HTML, strlen_P(INDEX_HTML));
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// OTA -- Over the Air (WiFi) upload
void OTA_setup() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("nsgamepad");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
        else // U_SPIFFS
        type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        })
  .onEnd([]() {
      Serial.println("\nEnd");
      })
  .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
  .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}

/* WiFiManager feedback callback */
void configModeCallback (WiFiManager *myWiFiManager) {
  clrscr();
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.print("Connect to NSGamepad with phone");
  RGBled = 0xFFFF00;
  FastLED.show();
  DBG_println("Entered config mode");
  DBG_println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DBG_println(myWiFiManager->getConfigPortalSSID());
}


void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
#if DEBUG_ON
  DBG_begin(115200);
  while (!Serial && (millis() < 3000)) delay(10);
  DBG_println(F("\n[usbnswa-debug] setup start"));
#else
  Serial.end();
#endif

  /* M5Stack AtomS3 display, RGB LED, and button */
#if !DIAG_NO_M5_UI
  DBG_println(F("[usbnswa-debug] init FastLED/button/display"));
  FastLED.addLeds<WS2812, LED_DI_PIN, GRB>(&RGBled, 1);
  RGBled = CRGB::Black;
  FastLED.show();

  // Place holder code for various button inputs. Replace or delete the code
  // as needed.
  button.attachClick([] {
#if 0
      clrscr();
      display.setTextColor(TFT_RED, TFT_BLACK);
      display.print("Button click");
      RGBled = CRGB::Red;
      FastLED.show();
#endif
      });
  button.attachDoubleClick([] {
#if 0
      clrscr();
      display.setTextColor(TFT_GREEN, TFT_BLACK);
      display.print("Button double click");
      RGBled = CRGB::Green;
      FastLED.show();
#endif
      });
  button.attachMultiClick([] {
      //reset settings - wipe credentials
      wm.resetSettings();

      clrscr();
      display.setTextColor(TFT_RED, TFT_BLACK);
      display.print("WiFi Password Erased");
      RGBled = CRGB::Red;
      FastLED.show();
      delay(1000);
      ESP.restart();
      });
  button.attachLongPressStart([] {
#if 0
      clrscr();
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      display.print("Button long press start");
      RGBled = 0x800080;
      FastLED.show();
#endif
      });
  button.attachDuringLongPress([] {
#if 0
      clrscr();
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      display.print("Button during long press");
      RGBled = 0x808000;
      FastLED.show();
#endif
      });
  button.attachLongPressStop([] {
#if 0
      clrscr();
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      display.print("Button long press stop");
      RGBled = 0x808080;
      FastLED.show();
#endif
      });
#else
  DBG_println(F("[usbnswa-debug] DIAG_NO_M5_UI: skip FastLED/button/display init"));
#endif

#if !DIAG_NO_M5_UI
  DBG_println(F("[usbnswa-debug] before setup_display"));
  setup_display();
  DBG_println(F("[usbnswa-debug] after setup_display"));
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.print("WiFi Connecting");
  RGBled = CRGB::Yellow;
  FastLED.show();
#else
  DBG_println(F("[usbnswa-debug] DIAG_NO_M5_UI: skip setup_display"));
#endif

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

#if NK_WIFI_MODE == NK_WIFI_MODE_HOME
  DBG_print(F("[switch2-keyboard] connecting to Wi-Fi SSID: "));
  DBG_println(F(NK_WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(NK_WIFI_SSID, NK_WIFI_PASS);
  bool res = false;
  for (uint8_t i = 0; i < NK_WIFI_CONNECT_ATTEMPTS; i++) {
    wl_status_t status = WiFi.status();
    DBG_print(F("[switch2-keyboard] WiFi.status="));
    DBG_println((int)status);
    if (status == WL_CONNECTED) {
      res = true;
      break;
    }
    delay(NK_WIFI_CONNECT_DELAY_MS);
  }
  DBG_println(F("[switch2-keyboard] after Wi-Fi connect attempt"));
#elif NK_WIFI_MODE == NK_WIFI_MODE_AP
  DBG_print(F("[switch2-keyboard] starting AP SSID: "));
  DBG_println(F(NK_AP_SSID));
  WiFi.mode(WIFI_AP);
  bool res = WiFi.softAP(NK_AP_SSID, NK_AP_PASS);
  DBG_print(F("[switch2-keyboard] AP IP: "));
  DBG_println(WiFi.softAPIP());
#else
#error "Unsupported NK_WIFI_MODE"
#endif

  if(!res) {
      DBG_println(F("Failed to connect"));
#if !DIAG_NO_M5_UI
      clrscr();
      display.setTextColor(TFT_RED, TFT_BLACK);
      display.print("WiFi failed to connect");
      RGBled = CRGB::Red;
      FastLED.show();
#endif
      delay(1000);
      ESP.restart();
      delay(1000);
  }

  DBG_println(F("[usbnswa-debug] before mDNS begin"));
  if (mdns.begin("nsgamepad")) {
    DBG_println(F("MDNS responder started"));
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    DBG_println(F("MDNS.begin failed"));
  }
  DBG_print(F("Connect to http://nsgamepad.local or http://"));
#if NK_WIFI_MODE == NK_WIFI_MODE_AP
  DBG_println(WiFi.softAPIP());
#else
  DBG_println(WiFi.localIP());
#endif
#if !DIAG_NO_M5_UI
  display.setTextColor(TFT_GREEN, TFT_BLACK);
  clrscr();
  display.print("nsgamepad.local or ");
#if NK_WIFI_MODE == NK_WIFI_MODE_AP
  display.print(WiFi.softAPIP());
#else
  display.print(WiFi.localIP());
#endif
  RGBled = CRGB::Green;
  FastLED.show();
#endif

  DBG_println(F("[usbnswa-debug] before OTA_setup"));
  OTA_setup();
  DBG_println(F("[usbnswa-debug] after OTA_setup"));

  DBG_println(F("[usbnswa-debug] before HTTP/WebSocket begin"));
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  DBG_println(F("[usbnswa-debug] after HTTP/WebSocket begin"));

  DBG_println(F("[usbnswa-debug] before Gamepad.begin"));
  Gamepad.begin();
  DBG_println(F("[usbnswa-debug] after Gamepad.begin, before USB.begin"));
  USB.begin();
  DBG_println(F("[usbnswa-debug] after USB.begin"));
}

void loop()
{
#if !DIAG_NO_M5_UI
  button.tick();
#endif
  ArduinoOTA.handle();
  webSocket.loop();
  server.handleClient();
#if !DIAG_DISABLE_GAMEPAD_LOOP
  Gamepad.loop();
#endif
}

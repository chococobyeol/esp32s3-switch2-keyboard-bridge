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

/* WiFi Network */

/* WiFiManager, global */
WiFiManager wm;

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

        // Send touch grid in JSON format
        Json_touch_grid(num, Gamepad_Page);
      }
      break;
    case WStype_TEXT:
      {
#if DIAG_VERBOSE_WS
        DBG_printf("[%u] get Text: [%d] %s \r\n", num, length, payload);
#endif

        StaticJsonDocument<96> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          DBG_print(F("deserializeJson() failed: "));
          DBG_println(error.f_str());
          return;
        }
        const char* event = doc["event"];
        if (strcmp(event, "keyup") == 0) {
          const int control = doc["code"];
          control_up(control);
        }
        else if (strcmp(event, "keydown") == 0) {
          const int control = doc["code"];
          control_down(control);
        }
        else {
          int row = doc["row"];
          if (row < 0) {
            DBG_printf("row negative %d\n", row);
            row = 0;
          }
          if (row >= MAX_ROWS) {
            DBG_printf("row too high %d\n", row);
            row = MAX_ROWS - 1;
          }

          int col = doc["col"];
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
          else if (strcmp(event, "touch end") == 0) {
            control_up(control);
          }
        }
        Gamepad.write();
      }
      break;
    case WStype_BIN:
#if DIAG_VERBOSE_WS
      DBG_printf("[%u] get binary length: %u\r\n", num, length);
#endif
      //      hexdump(payload, length);

      // echo data back to browser
      // webSocket.sendBIN(num, payload, length);
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

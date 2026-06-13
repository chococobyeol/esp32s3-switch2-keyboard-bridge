# ESP32-S3 Switch 2 Keyboard Bridge Firmware

This firmware turns an ESP32-S3 DevKit-style board into a Nintendo Switch 2 USB HID gamepad controlled from a browser over WebSocket.


## Compatibility

This firmware is currently tested only with Nintendo Switch 2. Nintendo Switch 1 compatibility is not tested and is not guaranteed.

## Current stable mode

- ESP32-S3 joins your home Wi-Fi.
- Browser opens the ESP32 web UI.
- Browser sends key/button events over WebSocket.
- ESP32-S3 sends USB HID reports to the Switch 2 dock.

```text
Mac/PC/phone browser -> home Wi-Fi router -> ESP32-S3 -> USB HID -> Switch 2 dock
```

## Private config

Copy the template and edit locally:

```bash
cp firmware/esp32s3_switch2_keyboard/config.local.example.h \
   firmware/esp32s3_switch2_keyboard/config.local.h
```

Then set:

```cpp
#define NK_WIFI_MODE NK_WIFI_MODE_HOME
#define NK_WIFI_SSID "YOUR_WIFI_SSID"
#define NK_WIFI_PASS "YOUR_WIFI_PASSWORD"
```

`config.local.h` is ignored by git and must not be committed.

## Optional direct AP mode

Direct AP mode is scaffolded but not the default:

```cpp
#define NK_WIFI_MODE NK_WIFI_MODE_AP
#define NK_AP_SSID "NSGamepad"
#define NK_AP_PASS ""
```

In AP mode your laptop/phone must join the ESP32 Wi-Fi network, so Wi-Fi Internet may disconnect unless you have Ethernet or another network path.

## Arduino CLI build/upload

Install dependencies first:

```bash
arduino-cli core install esp32:esp32@3.3.10
arduino-cli lib install WebSockets ArduinoJson WiFiManager OneButton FastLED M5GFX
```

Compile/upload for ESP32-S3 N16R8 DevKit-style board:

```bash
FQBN='esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default,UploadMode=default,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi'

arduino-cli compile --clean --fqbn "$FQBN" firmware/esp32s3_switch2_keyboard
arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn "$FQBN" firmware/esp32s3_switch2_keyboard
```

## Browser test

After boot, open the IP printed on serial output, usually like:

```bash
open http://192.168.x.x/
```

Expected:

- Touch Gamepad grid renders.
- DevTools shows `websock onopen`.
- Keyboard WASD/arrow keys and grid buttons send inputs.

## Stability notes

This firmware intentionally disables:

- M5Stack AtomS3 display/LED/button initialization (`NK_DISABLE_M5_UI=1`) because the tested board is a generic ESP32-S3 DevKit.
- Periodic `Gamepad.loop()` reports (`NK_DISABLE_GAMEPAD_LOOP=1`) because periodic HID reports destabilized HTTP/WebSocket on this board/core combination.

Instead, the web UI repeats active inputs every 20ms and keeps short clicks pressed for at least 90ms.

# esp32s3-switch2-keyboard-bridge

ESP32-S3 firmware for controlling a Nintendo Switch 2 from a browser keyboard UI over Wi-Fi/WebSocket.

```text
Browser keyboard UI -> Wi-Fi/WebSocket -> ESP32-S3 -> USB HID gamepad -> Nintendo Switch 2 dock
OBS Browser Source overlay -> same WebSocket state relay -> transparent key display
```

## Compatibility

This project is currently tested only with Nintendo Switch 2.

Nintendo Switch 1 compatibility is not tested and is not guaranteed.

## Current status

Validated on a generic ESP32-S3 DevKitC-1-compatible N16R8 dual USB-C board:

- ESP32-S3 joins home Wi-Fi.
- Browser opens the ESP32-hosted keyboard control UI.
- WebSocket delivers normalized input events and live UI state.
- ESP32-S3 sends USB HID gamepad reports to a Nintendo Switch 2 dock.
- ESP32 station-mode Wi-Fi sleep is disabled in home Wi-Fi mode to reduce input latency spikes.
- A separate `?overlay=1` page renders a transparent mapped-key overlay for OBS Browser Source.

## UI workflow

Open the IP printed by serial output, for example:

```text
http://192.168.x.x/
```

The root page shows a full keyboard layout. Click a key, assign a Switch control, and use the import/export box to save or restore the active keymap JSON.

Default starter mapping includes WASD for left stick, arrow keys for right stick, and a small set of face/shoulder buttons. The mapping is intentionally one keyboard key to one Switch control; macros and automated input sequences are out of scope.

### Stick sensitivity

Left and right stick sensitivity are independent persistent toggles:

- left-stick toggle default: left Control
- right-stick toggle default: left Alt
- full mode sends full deflection
- half mode sends the configured half-stroke percentage

The half-stroke percentage and toggle keys can be edited in the UI.

### OBS overlay

Use this URL as an OBS Browser Source:

```text
http://192.168.x.x/?overlay=1
```

The overlay is transparent, compact, and mapped-only. It receives the current keymap, pressed state, and stick modes through the ESP32 WebSocket state relay, so it can run in a separate OBS browser context. The current MVP assumes one active control page as the UI owner; if multiple control pages are open, the most recent valid `state_snapshot` becomes the owner. Overlay clients are display-only.

## Firmware

Main sketch:

```text
firmware/esp32s3_switch2_keyboard/esp32s3_switch2_keyboard.ino
```

Private Wi-Fi credentials are kept out of git:

```bash
cp firmware/esp32s3_switch2_keyboard/config.local.example.h \
   firmware/esp32s3_switch2_keyboard/config.local.h
```

Edit `config.local.h` locally. Do not commit it.

## Build / upload

Install Arduino CLI dependencies:

```bash
arduino-cli core install esp32:esp32@3.3.10
arduino-cli lib install WebSockets ArduinoJson WiFiManager OneButton FastLED M5GFX
```

Compile/upload:

```bash
FQBN='esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default,UploadMode=default,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi'

arduino-cli compile --clean --fqbn "$FQBN" firmware/esp32s3_switch2_keyboard
arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn "$FQBN" firmware/esp32s3_switch2_keyboard
```

## Not in this MVP

- Bluetooth transport
- Hosted/static web app
- Wi-Fi captive-portal redesign
- Macro/combo/chord automation
- Switch 1 compatibility guarantee

## Attribution

This project was built from experiments based on:

- <https://github.com/touchgadget/usbnswa>
- <https://github.com/esp32beans/switch_ESP32>

Vendored `switch_ESP32` files retain their MIT license notice in the source headers.

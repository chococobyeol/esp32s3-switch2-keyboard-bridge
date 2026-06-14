# esp32s3-switch2-keyboard-bridge

ESP32-S3 firmware for controlling a Nintendo Switch 2 from a browser keyboard UI over Wi-Fi/WebSocket.

```text
Browser keyboard UI -> Wi-Fi/WebSocket -> ESP32-S3 -> USB HID gamepad -> Nintendo Switch 2 dock
OBS Browser Source overlay -> same WebSocket state relay -> transparent key display
```

The browser UI is embedded in the firmware. After flashing, users open the ESP32 address directly; no hosted static control site is required.

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

## Hardware requirements

Recommended:

- ESP32-S3 development board with native USB device support.
- Nintendo Switch 2 dock.
- USB cable from ESP32-S3 native USB port to the Switch 2 dock.
- A Mac/PC/phone browser on the same Wi-Fi network as the ESP32-S3.

Notes:

- ESP32-S3 is required because this firmware uses native USB HID.
- Boards with only a USB-to-serial/UART bridge and no native USB device path are not suitable for the Switch-side HID connection.
- Dual-USB boards usually expose one UART/programming port and one native USB port. Upload can be done through the UART port, but the Switch dock must see the native USB port.

## Firmware layout

Arduino treats the whole sketch folder as one firmware project. Do **not** copy/upload only the `.ino` file.

Required sketch folder:

```text
firmware/esp32s3_switch2_keyboard/
├── esp32s3_switch2_keyboard.ino
├── config.h
├── index_html.h
├── gamepad_grid.h
├── switch_ESP32.h
└── switch_ESP32.cpp
```

Local-only Wi-Fi credentials go in this ignored file:

```text
firmware/esp32s3_switch2_keyboard/config.local.h
```

## Quick setup / upload

### 1. Install Arduino CLI

On macOS with Homebrew:

```bash
brew install arduino-cli
arduino-cli version
```

### 2. Add the ESP32 board package

If this is a fresh Arduino CLI install:

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@3.3.10
```

### 3. Install Arduino libraries

```bash
arduino-cli lib update-index
arduino-cli lib install WebSockets ArduinoJson WiFiManager OneButton FastLED M5GFX
```

### 4. Create private Wi-Fi config

Copy the template:

```bash
cp firmware/esp32s3_switch2_keyboard/config.local.example.h \
   firmware/esp32s3_switch2_keyboard/config.local.h
```

Edit `firmware/esp32s3_switch2_keyboard/config.local.h`:

```cpp
#pragma once

#define NK_WIFI_MODE NK_WIFI_MODE_HOME
#define NK_WIFI_SSID "YOUR_WIFI_SSID"
#define NK_WIFI_PASS "YOUR_WIFI_PASSWORD"
```

`config.local.h` is ignored by git. Do not commit it.

### 5. Find the upload port

Connect the ESP32-S3 to the computer, then run:

```bash
arduino-cli board list
ls /dev/cu.*
```

Common macOS examples:

```text
/dev/cu.usbserial-0001      # UART/programming bridge on many dual-USB boards
/dev/cu.usbmodemXXXXXXXX    # native USB CDC on some boards/modes
```

Use the actual port from your machine in the upload command.

### 6. Compile and upload

For the tested ESP32-S3 N16R8 DevKit-style board:

```bash
FQBN='esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default,UploadMode=default,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi'

arduino-cli compile --clean --fqbn "$FQBN" firmware/esp32s3_switch2_keyboard

arduino-cli upload \
  -p /dev/cu.usbserial-0001 \
  --fqbn "$FQBN" \
  firmware/esp32s3_switch2_keyboard
```

Change `-p /dev/cu.usbserial-0001` if your board appears under a different port.

For other ESP32-S3 boards, the FQBN may need different `FlashSize`, `PSRAM`, or board target options. The important part is that native USB is available for HID.

### 7. Open the control UI

After boot, monitor serial output to see the assigned IP:

```bash
arduino-cli monitor -p /dev/cu.usbserial-0001 -c baudrate=115200
```

Look for a line like:

```text
Connect to http://nsgamepad.local or http://192.168.x.x
```

Open the page in a browser:

```text
http://192.168.x.x/
```

If mDNS works on your network, this may also work:

```text
http://nsgamepad.local/
```

## Switch 2 connection

After upload and Wi-Fi connection:

1. Keep the ESP32-S3 powered.
2. Connect the ESP32-S3 native USB port to the Nintendo Switch 2 dock.
3. Wake the Switch 2 normally with an official controller or the console power button.
4. Open the ESP32 web UI from the browser.
5. Press mapped keys and verify input in a game or the Switch controller test screen.

Sleep wake is not a supported goal for this bridge. The Home mapping can work while the Switch 2 is awake, but generic USB HID wake-from-sleep is not expected to behave like an official Switch 2 controller.

## UI workflow

Open the IP printed by serial output, for example:

```text
http://192.168.x.x/
```

The root page shows a full keyboard layout. Click a key, assign a Switch control, and use the import/export box to save or restore the active keymap JSON.

Default starter mapping includes WASD for left stick, arrow keys for right stick, and a small set of face/shoulder buttons. The mapping is intentionally one keyboard key to one Switch control; macros and automated input sequences are out of scope for the current firmware.

### Input tuning

Important UI settings:

- **Input state interval**: default `20ms` latest-state refresh while a key is held.
- **Button hold ms**: minimum press duration for short taps. The UI allows `40ms` to `250ms`; lower values are more responsive but can miss taps.
- **Half stroke percent**: stick half-mode strength.

The current practical latency baseline depends on keeping ESP32 station-mode Wi-Fi sleep disabled, which is the firmware default.

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

The overlay is transparent, compact, mapped-only, and display-only. It receives the current keymap, pressed state, and stick modes through the ESP32 WebSocket state relay, so it can run in a separate OBS browser context.

Use one normal control page as the active controller. Multiple control pages can fight over state; the project intentionally does not add complex tab ownership logic.

## Troubleshooting

### Upload succeeds but Switch 2 does not respond

- Confirm the Switch dock is connected to the ESP32-S3 native USB port, not only the UART/programming port.
- Confirm the Switch 2 is awake.
- Confirm the browser UI shows `WS open`.
- Try the Switch controller input test screen.

### Browser cannot open the UI

- Check serial output for the actual IP.
- Make sure the browser device is on the same Wi-Fi network.
- Try the IP address directly instead of `nsgamepad.local`.
- If Wi-Fi credentials changed, update `config.local.h` and re-upload, or use AP mode experimentally.

### Input suddenly becomes very delayed

- Check `RTT` and `Pending ack` in the UI.
- If RTT jumps to seconds, the local Wi-Fi/WebSocket path is stalling.
- Keep only one control page open; OBS should use `?overlay=1` only.
- Keep `NK_WIFI_DISABLE_SLEEP=1` unless intentionally testing power-saving behavior.

### Sensitive files before publishing

Before pushing to GitHub, check:

```bash
git status --short
git check-ignore -v firmware/esp32s3_switch2_keyboard/config.local.h
```

`config.local.h` should be ignored and should not appear as a tracked file.

## Optional direct AP mode

Home Wi-Fi mode is the stable default. Direct AP mode is available but less convenient because the computer/phone must join the ESP32 Wi-Fi network and may lose Internet on Wi-Fi.

In `config.local.h`:

```cpp
#define NK_WIFI_MODE NK_WIFI_MODE_AP
#define NK_AP_SSID "NSGamepad"
#define NK_AP_PASS ""
```

Then recompile and upload.

## Not in this MVP

- Bluetooth transport
- Hosted/static control web app
- Wi-Fi captive-portal redesign
- Macro/combo/chord automation
- Switch 1 compatibility guarantee
- Switch 2 sleep wake support

## Attribution

This project was built from experiments based on:

- <https://github.com/touchgadget/usbnswa>
- <https://github.com/esp32beans/switch_ESP32>

Vendored `switch_ESP32` files retain their MIT license notice in the source headers.

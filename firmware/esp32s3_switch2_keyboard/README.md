# ESP32-S3 Switch 2 Keyboard Bridge Firmware

This firmware turns an ESP32-S3 DevKit-style board into a Nintendo Switch 2 USB HID gamepad controlled from a browser over WebSocket.

## Compatibility

This firmware is currently tested only with Nintendo Switch 2. Nintendo Switch 1 compatibility is not tested and is not guaranteed.

## Current stable mode

- ESP32-S3 joins your home Wi-Fi.
- Browser opens the ESP32 web UI.
- Browser sends normalized input events over WebSocket.
- ESP32-S3 sends USB HID reports to the Switch 2 dock.

```text
Mac/PC/phone browser -> home Wi-Fi router -> ESP32-S3 -> USB HID -> Switch 2 dock
```

## Browser UI

Open the IP printed on serial output, usually like:

```bash
open http://192.168.x.x/
```

Expected root-page behavior:

- A keyboard-oriented control surface renders instead of the old touch grid.
- The UI shows WebSocket state, reconnect count, buffered bytes, last input, and RTT.
- Click a keyboard key to assign or clear a Switch control.
- Keymap/settings are cached in browser `localStorage` and can be exported/imported as JSON.
- Pressed keys are highlighted in the control page.

## OBS overlay mode

Open the overlay URL:

```text
http://192.168.x.x/?overlay=1
```

The overlay page is transparent and mapped-only for OBS Browser Source. It is display-only: keyboard events are ignored in overlay mode. A normal control page publishes live state snapshots/patches, and the firmware relays the latest state to overlay clients. The current MVP assumes one active control page as the UI owner; the most recent valid `state_snapshot` claims ownership, and later `state_patch` messages must come from that owner.

## Stick sensitivity

The UI supports independent persistent full/half modes for left and right sticks.

Defaults:

- left-stick mode toggle: `ControlLeft`
- right-stick mode toggle: `AltLeft`
- half-stroke: 50%
- button hold: 120ms
- input state interval: 20ms
- station-mode Wi-Fi sleep: disabled by default for lower input latency

Full mode sends full stick deflection. Half mode sends the configured half-stroke axis value. The toggle keys and half percentage are editable in the browser UI.

In half mode, diagonal stick input is normalized by `1/sqrt(2)` when both axes of the same stick are active. For example, with 70% half stroke, a single-axis input sends about 70% deflection, while a diagonal sends about 49% on X and 49% on Y so the radial stick magnitude remains about 70% instead of becoming nearly full stroke.

## Input state tuning

The browser control page sends latest-state frames instead of queueing every repeated `down` event. A key change is sent immediately, then the currently pressed controls are refreshed at the configured interval: `20ms` (default), `40ms`, `60ms`, `80ms`, `100ms`, or `off` for change-only mode. Each state frame replaces the full HID state on the ESP32-S3, so stale queued direction events are not intentionally replayed. Exported logs can compare sampled `Ack avg/max`, `Pending ack`, and `Input sent/drop`; actual game/OBS visual latency still requires video/frame-based checking.

## WebSocket protocol

The firmware accepts a versioned JSON envelope:

```json
{ "v": 1, "type": "input", "action": "down", "control": 20, "id": "LeftStickUp", "value": 0 }
```

Supported message types:

- `input` — legacy normalized HID press/release event.
- `control_state` — current full input state; firmware resets HID controls and applies this latest state in one `Gamepad.write()`.
- `state_snapshot` — full browser UI state for overlay sync; includes an `owner` session id.
- `state_patch` — visible-state update for overlay sync; must match the active `owner`.
- `state_request` — overlay asks firmware to resend latest state.
- `ping` / `pong` — browser RTT telemetry.

Firmware validates protocol version, owner shape, known keyboard codes, known control IDs, stick modes, half-stroke range, and pressed-key shape before relaying UI state. Legacy `keydown`, `keyup`, `touch start`, `touch end`, and `touch cancel` messages are still accepted during migration.

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

## Latency notes

In home Wi-Fi mode the firmware disables ESP32 station-mode Wi-Fi sleep (`NK_WIFI_DISABLE_SLEEP=1`) before connecting. This reduces WebSocket input latency spikes that can otherwise feel like 100ms+ controller lag. The bridge is normally USB-powered, so the extra power draw is acceptable for this use case.

## Stability notes

This firmware intentionally disables:

- M5Stack AtomS3 display/LED/button initialization (`NK_DISABLE_M5_UI=1`) because the tested board is a generic ESP32-S3 DevKit.
- Periodic `Gamepad.loop()` reports (`NK_DISABLE_GAMEPAD_LOOP=1`) because periodic HID reports destabilized HTTP/WebSocket on this board/core combination.

Instead, the web UI sends latest-state frames at the configured interval (20ms by default) and keeps short clicks pressed for at least 120ms. This avoids replaying a long backlog of stale direction events.

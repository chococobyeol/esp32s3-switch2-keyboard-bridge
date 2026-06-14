# Wi-Fi sleep latency feedback — 2026-06-15

## Context

The bridge is currently tested in this path:

```text
Browser keyboard UI -> home Wi-Fi/WebSocket -> ESP32-S3 -> USB HID -> Nintendo Switch 2 dock
```

The repeated-input backlog issue had already been reduced by switching the browser protocol to latest-state frames. A remaining problem was that even a single key press still felt noticeably slower than a normal controller.

## Finding

The ESP32-S3 station-mode Wi-Fi power-save behavior was a likely source of packet wake/scheduling delay. On an input bridge, that delay is directly visible as controller lag. The firmware now disables Wi-Fi sleep after entering station mode:

```cpp
WiFi.mode(WIFI_STA);
WiFi.setSleep(false);
WiFi.begin(...);
```

This is intentionally applied only to the home Wi-Fi station path. It is not a secret-bearing change and does not alter local Wi-Fi credentials.

## User feedback

After flashing the firmware with Wi-Fi sleep disabled, the keyboard response felt substantially better. This suggests the previous 100–200ms-feeling delay was not primarily from OBS/capture latency or the Switch 2 game itself, because the same capture path felt acceptable with an official controller.

## Expected impact

Disabling Wi-Fi sleep should reduce latency spikes in the Mac/PC/phone browser -> ESP32 WebSocket segment. It does not make the whole path equivalent to a native controller, but it makes the current WebSocket + USB HID architecture more plausible as the main implementation path.

Target telemetry after this change:

- `Ack avg`: roughly 10–30ms when the network is healthy
- `Ack max`: should avoid frequent 100ms+ spikes during normal play
- `RTT`: should stay low and stable enough that visual input lag is not dominated by Wi-Fi wake delays

Actual game-visible latency still requires real play/video testing because the browser log only measures browser-to-firmware acknowledgement, not Switch/game-frame response.

## Caveats

- Wi-Fi sleep disabled may increase ESP32 power usage. This is acceptable for docked USB-powered play.
- Router/AP behavior can still create latency spikes.
- OBS overlay rendering is separate from input transport; overlay issues should not be used as the only input-latency signal.
- If latency remains high after this change, the next meaningful comparison is a wired Mac -> ESP32 serial -> USB HID path, or the Raspberry Pi Bluetooth Classic controller-emulation path.

## Next validation

1. Play for 5–10 minutes with the same OBS/capture setup.
2. Record `Ack avg`, `Ack max`, `RTT`, and subjective feel.
3. Stress quick direction changes and single-tap A/B button presses.
4. Keep this setting enabled unless a specific board/router combination proves unstable.

# BLE CLI Current Status

## Snapshot

The project is a working ESP32-first BLE CLI built on PlatformIO, Arduino, NimBLE-Arduino, and M5Unified.

It can:

- scan for BLE devices
- select and connect to a device
- inspect the HID service
- read HID report maps
- subscribe to HID input notifications
- suppress repeated monitor lines with dedup
- reboot the ESP32 cleanly from the shell

## Current Hardware / Tooling

- Board target: `m5stick-c`
- Framework: Arduino
- Build tool: PlatformIO
- BLE stack: NimBLE-Arduino
- Display: M5Unified

## Important Files

- [platformio.ini](C:\Users\Dave\Documents\Codex\2026-06-10\looing-for-a-ble-pairing-to\platformio.ini)
- [src/main.cpp](C:\Users\Dave\Documents\Codex\2026-06-10\looing-for-a-ble-pairing-to\src\main.cpp)
- [src/app/BleCliApp.cpp](C:\Users\Dave\Documents\Codex\2026-06-10\looing-for-a-ble-pairing-to\src\app\BleCliApp.cpp)
- [src/backend/esp32/Esp32BleBackend.cpp](C:\Users\Dave\Documents\Codex\2026-06-10\looing-for-a-ble-pairing-to\src\backend\esp32\Esp32BleBackend.cpp)

## Working Commands

- `scan start`
- `scan stop`
- `devices`
- `select device <index|address>`
- `connect`
- `disconnect`
- `services`
- `hid map`
- `hid chars`
- `hid reports`
- `monitor`
- `status`
- `reboot`
- `restart`
- `back`
- `root`

## Shell Behavior

- The prompt shows the current selected device and context path.
- Backspace and delete remove characters from the current input line.
- `reboot` blanks the screen for 1.5 seconds before restarting the ESP32.
- `monitor` prints raw HID notification bytes, but repeat suppression is still enabled.
- `monitor` is currently the simpler stable path, and it is receiving shorter but valid key and trackpad activity again.

## HID Findings So Far

For the current keyboard/gamepad-style devices, the HID service usually exposes:

- `0x1812` HID service
- `0x2A22` boot keyboard input
- several `0x2A4D` report characteristics
- `0x2A4B` report map
- `0x2A4E` protocol mode
- `0x2A4C` control point
- `0x2A19` battery level on some devices

We have observed:

- report ID `1` is often keyboard input
- report ID `3` is often trackpad/pointer-style input
- report ID `2` is often consumer/media input
- report ID `6` is often system/power input

## Current BLE Behavior

What works:

- the connection path succeeds
- the HID service is discovered
- report descriptors can be read
- monitor subscriptions can be established
- repeated packets are deduped so the terminal stays readable
- key events and trackpad events are both being received again

What is still flaky:

- some devices only start producing notifications after `hid map` has been run first
- some runs appear quiet directly after `connect`
- serial prompt text and NimBLE log messages can interleave and make the terminal output messy

## Current Monitor Mode

- default `monitor`:
  - subscribes to HID streams
  - keeps repeat suppression enabled
  - no longer forces protocol mode or control-point writes
  - no longer uses the raw bypass mode
  - currently returns shorter packets than earlier experiments, but the packets are valid and still include key and trackpad activity

## Remaining Open Issues

- `monitor` can still appear quiet on some devices directly after connect
- stream labeling may still need refinement if we decide to re-add it later
- terminal output can become interleaved with NimBLE logging
- some HID devices may still need a more explicit protocol-mode or report-selection command

## Near-Term Next Steps

1. Keep validating the current simple `monitor` path on this device family.
2. If needed, add a lightweight non-invasive report summary command.
3. Reintroduce stream tags only if they do not disturb monitoring.
4. Split stream handling further if the trackpad or button paths need separate handling.

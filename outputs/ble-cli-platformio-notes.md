# PlatformIO Basis for ESP32 BLE CLI

## Why PlatformIO

PlatformIO is a good base for this project because it gives you:

- reproducible toolchains
- per-project dependency/version pinning
- a clean build/upload workflow
- easy environment switching for different ESP32 boards

For an ESP32 BLE host tool, that is a better fit than a loose Arduino IDE workflow.

## Recommended Setup

Use PlatformIO as the project root and target the Arduino framework for ESP32.

Suggested structure:

```text
platformio.ini
src/
  main.cpp
  app/
  core/
  backend/
  profiles/
  output/
include/
lib/
test/
```

## Versioning Strategy

Pin the important pieces in `platformio.ini` so the project stays stable:

- ESP32 platform package version
- Arduino framework version, if needed
- NimBLE-Arduino dependency version

That gives you repeatable builds without depending on whatever happens to be installed globally.

## Example `platformio.ini` Shape

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
lib_deps =
    h2zero/NimBLE-Arduino
build_flags =
    -D CORE_DEBUG_LEVEL=3
```

## Workflow Notes

- Use `pio run` for builds
- Use `pio run --target upload` for flashing
- Use `pio device monitor` for the serial console
- Keep the BLE CLI shell available through serial input/output

## Practical Benefit For This Project

PlatformIO lets us keep the ESP32/NimBLE work organized while still using Arduino-style APIs.

That means:

- faster iteration
- cleaner dependency control
- less environment drift
- better long-term repeatability for BLE debugging


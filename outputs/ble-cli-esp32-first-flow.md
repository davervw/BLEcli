# ESP32 First BLE Flow

## Purpose

This document captures the first concrete command flow for the ESP32 + NimBLE-Arduino BLE CLI.

The goal is to validate the full path from discovery to live HID events with the smallest useful amount of moving parts.

## First-Run Flow

1. Boot the ESP32 and start the CLI shell.
2. Initialize NimBLE-Arduino.
3. List available adapters or confirm the built-in controller is ready.
4. Start scanning for nearby BLE peripherals.
5. Show discovered devices with address, name, RSSI, and optional service hints.
6. Select the target keyboard or gamepad.
7. Connect to the device.
8. Pair if the device requires security.
9. Discover services.
10. Find the HID service, usually `0x1812`.
11. Read the HID Report Map characteristic if needed.
12. Find the input report characteristic.
13. Subscribe to notifications on the input report.
14. Stream raw events to the shell.
15. Pass payloads through the HID decoder when possible.

## Suggested Shell Commands

```text
scan start
devices
select device AA:BB:CC:DD:EE:FF
connect
pair
services
select service 1812
chars
select char 2a4d
subscribe on
monitor
```

## Internal Event Sequence

The implementation should follow this order:

1. Scan callback emits a normalized device record.
2. Session stores the selected device.
3. Connection callback updates connection state.
4. GATT discovery populates the service and characteristic list.
5. Subscription enables notifications on the input report.
6. Notification callback forwards the raw payload to the event bus.
7. HID decoder attempts to interpret the payload.
8. Output layer prints raw and decoded forms.

## Early Success Criteria

The first milestone is successful when the tool can:

- discover a real BLE keyboard or gamepad
- connect and subscribe without crashing
- show incoming bytes in the terminal
- keep the shell responsive during live input

## Next Implementation Step

After this flow works, the next useful additions are:

- report map retrieval
- better keyboard decoding
- basic gamepad state decoding
- pairing/bonding diagnostics


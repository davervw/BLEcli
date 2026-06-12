# ESP32 NimBLE-Arduino Module and Class Layout

## Purpose

This document sketches a practical ESP32-first architecture for a BLE investigation CLI that uses NimBLE-Arduino as the host stack.

The goal is to keep the design simple enough to implement quickly while still separating concerns cleanly enough to support keyboards, gamepads, and other BLE peripherals.

## Recommended Scope

- Primary target: ESP32
- Primary BLE stack: NimBLE-Arduino
- Primary interaction style: interactive shell plus optional scripted commands
- Primary device classes: BLE HID keyboards and gamepads

This layout assumes you want to inspect devices, pair with them, subscribe to notifications, and decode raw input reports using the Arduino-style NimBLE APIs.

## Top-Level Structure

```text
src/
  app/
    main.*
    cli_shell.*
    prompt.*
    command_router.*
  core/
    session.*
    context_stack.*
    event_bus.*
    device_registry.*
    output_mode.*
  backend/
    esp32/
      esp32_ble_backend.*
      nimble_adapter.*
      nimble_device.*
      nimble_gatt.*
      nimble_security.*
      nimble_scan.*
      nimble_callbacks.*
  profiles/
    hid/
      hid_profile.*
      hid_report_map.*
      hid_report_parser.*
      keyboard_decoder.*
      gamepad_decoder.*
    generic/
      raw_decoder.*
      hex_decoder.*
  output/
    table_renderer.*
    json_renderer.*
    text_renderer.*
    log_sink.*
tests/
```

## Runtime Flow

The intended flow is:

1. Shell receives a command
2. Command router maps it to an action
3. Session resolves the current context
4. ESP32 NimBLE backend executes the BLE operation
5. Result is emitted to the event bus
6. Output layer renders human-readable and/or JSON output
7. HID profile decoder interprets notifications when possible

This keeps the UI responsive and lets raw BLE data and decoded HID data coexist.

## Core Classes

### `CliShell`

Owns the interactive terminal loop.

Responsibilities:

- Read user input
- Display the prompt
- Print command results
- Support `help`, `back`, `root`, `exit`
- Keep the shell usable during live event streaming

### `CommandRouter`

Parses input into command objects and dispatches them.

Responsibilities:

- Parse tokens
- Validate command syntax
- Route to the correct handler
- Separate global commands from context-specific commands

Suggested methods:

- `parse(line)`
- `dispatch(command, session)`
- `suggestCompletions(prefix)` if tab completion is added later

### `Session`

Represents the current user-visible state.

Responsibilities:

- Track active adapter
- Track active device
- Track active service
- Track active characteristic
- Track active descriptor
- Track pairing and connection state
- Hold current output mode
- Hold the selected decoder mode

### `ContextStack`

Maintains the breadcrumb-style selection path.

Responsibilities:

- Push and pop contexts
- Resolve `back` and `root`
- Render the current shell prompt
- Ensure commands apply to the right object

Example path:

```text
/adapter/0 /device/AA:BB:CC:DD:EE:FF /service/1812 /char/2a4d
```

### `EventBus`

Normalizes live BLE and decoding events.

Responsibilities:

- Receive notifications from the backend
- Broadcast raw and decoded events
- Timestamp events
- Feed logs and output renderers

### `DeviceRegistry`

Caches discovered devices and known metadata.

Responsibilities:

- Store scan results
- Update RSSI and name data
- Track known bond state
- Retain service summaries

## ESP32 Backend Classes

### `Esp32BleBackend`

This is the high-level BLE backend facade used by the rest of the app.

Responsibilities:

- Start and stop scans
- Connect and disconnect from devices
- Initiate pairing and bonding
- Read and write GATT values
- Subscribe and unsubscribe to notifications
- Trigger service and characteristic discovery

This class should expose the app-facing interface and hide NimBLE-Arduino details.

Suggested interface:

```text
scanStart()
scanStop()
listDevices()
connect(deviceId)
disconnect(deviceId)
pair(deviceId)
discoverServices(deviceId)
discoverCharacteristics(serviceId)
discoverDescriptors(characteristicId)
readCharacteristic(characteristicId)
writeCharacteristic(characteristicId, bytes)
subscribeCharacteristic(characteristicId)
unsubscribeCharacteristic(characteristicId)
```

### `NimbleAdapter`

Wraps the local BLE controller and scan lifecycle through `NimBLEDevice`, `NimBLEScan`, and related Arduino APIs.

Responsibilities:

- Initialize NimBLE
- Manage adapter power state
- Configure scan parameters
- Collect scan results

### `NimbleScan`

Handles active scanning and result normalization.

Responsibilities:

- Receive advertisements
- Extract address, RSSI, name, and service UUID hints
- Match optional scan filters
- Emit structured device discovery events

### `NimbleDevice`

Represents a connected peripheral and its session state.

Responsibilities:

- Store connection handle or peer identifier
- Track negotiated MTU if exposed
- Track security state
- Hold discovered GATT hierarchy

### `NimbleGatt`

Handles GATT discovery and read/write/subscribe operations using `NimBLEClient`, `NimBLERemoteService`, `NimBLERemoteCharacteristic`, and descriptor objects.

Responsibilities:

- Service discovery
- Characteristic discovery
- Descriptor discovery
- Characteristic reads and writes
- CCCD writes for notifications and indications

### `NimbleSecurity`

Encapsulates pairing and bonding.

Responsibilities:

- Initiate pairing
- Track authentication state
- Track encryption state
- Store or load bond information if supported by the platform layer

## HID Profile Classes

### `HidProfile`

Orchestrates HID-specific discovery and decoding.

Responsibilities:

- Find the HID service
- Locate the Report Map characteristic
- Locate input report characteristics
- Subscribe to report notifications
- Route report payloads to the correct decoder

### `HidReportMap`

Represents the descriptor/report map data.

Responsibilities:

- Store raw report map bytes
- Parse usage pages and report IDs if needed
- Provide lookup data for decoders

### `HidReportParser`

Common parser for raw HID reports.

Responsibilities:

- Split report ID from payload if present
- Normalize report packets
- Pass clean payloads to keyboard or gamepad decoders

### `KeyboardDecoder`

Specializes in keyboard input reports.

Responsibilities:

- Decode modifier bits
- Decode keycode arrays
- Detect pressed and released keys
- Show raw and interpreted state

### `GamepadDecoder`

Specializes in gamepad input reports.

Responsibilities:

- Decode button bits
- Decode axes if the report structure is known
- Show raw and interpreted state

### `RawDecoder`

Fallback decoder for unknown or unsupported payloads.

Responsibilities:

- Print hex bytes
- Preserve timestamps
- Keep data visible when decoding fails

## Output Classes

### `TableRenderer`

Formats device lists, service lists, and characteristic lists.

### `TextRenderer`

Formats status, errors, and live event summaries.

### `JsonRenderer`

Serializes events and query results for script use.

### `LogSink`

Writes persistent logs of scan results, pairing events, and notifications.

## Suggested Command-to-Class Mapping

| Command | Primary Class |
| --- | --- |
| `scan start` | `Esp32BleBackend`, `NimbleScan` |
| `devices` | `DeviceRegistry`, `TableRenderer` |
| `select device ...` | `Session`, `ContextStack` |
| `connect` | `Esp32BleBackend`, `NimbleDevice` |
| `pair` | `NimbleSecurity` |
| `services` | `NimbleGatt` |
| `select service ...` | `Session`, `ContextStack` |
| `chars` | `NimbleGatt` |
| `subscribe on` | `NimbleGatt`, `EventBus` |
| `monitor` | `EventBus`, `HidProfile` |
| `hid map` | `HidProfile`, `HidReportMap` |
| `hid decode keyboard` | `KeyboardDecoder` |
| `hid decode gamepad` | `GamepadDecoder` |

## Event Model

The backend should emit structured events rather than printing directly.

Suggested event types:

- `scan_result`
- `device_connected`
- `device_disconnected`
- `pairing_started`
- `pairing_succeeded`
- `pairing_failed`
- `service_discovered`
- `characteristic_discovered`
- `descriptor_discovered`
- `notification_received`
- `decode_result`
- `error`

This makes it easy to render the same event in multiple ways.

## Data Objects

Use small value objects for BLE entities:

- `AdapterInfo`
- `DeviceInfo`
- `ServiceInfo`
- `CharacteristicInfo`
- `DescriptorInfo`
- `SubscriptionInfo`
- `ReportInfo`
- `DecodedInputEvent`

These should be simple data carriers so the backend and UI stay loosely coupled.

## Minimal Dependency Boundaries

Try to keep these dependencies one-way:

- `app` depends on `core` and `output`
- `core` depends on `backend` interfaces and `profiles`
- `backend/esp32` depends on NimBLE
- `profiles/hid` depends only on raw event/data objects
- `output` depends on data objects but not on NimBLE directly

This prevents BLE stack code from leaking into the shell or formatting layers.

## Implementation Sequence

Recommended order:

1. Shell loop and command router
2. Session and context model
3. ESP32 NimBLE-Arduino scan and device list
4. Connect and disconnect
5. Pairing and security state
6. GATT discovery
7. Notification subscription
8. Raw event rendering
9. HID report map retrieval
10. Keyboard decoder
11. Gamepad decoder

## Practical Recommendation

If you keep only one abstraction boundary, make it the `Esp32BleBackend` interface. That gives you enough structure to keep the CLI sane, but not so much abstraction that the project becomes harder to build than the BLE stack itself.

## Arduino-Style Implementation Notes

With NimBLE-Arduino, the design should lean on callbacks rather than polling.

Preferred callback points:

- `NimBLEAdvertisedDeviceCallbacks` for scan results
- `NimBLEClientCallbacks` for connect and disconnect events
- `NimBLERemoteCharacteristicCallbacks` for notifications and indications

That suggests the following event flow:

1. Scan callback normalizes discovered devices
2. Client callback updates connection state
3. Characteristic callback forwards raw payloads to the event bus
4. Profile decoders process the payload asynchronously or in a lightweight handler

## Arduino Loop Model

The CLI should still feel like a shell, but under the hood the Arduino loop should be kept responsive.

Recommended split:

- `setup()`
  - Initialize serial console
  - Initialize NimBLE
  - Initialize CLI session state
- `loop()`
  - Pump command input
  - Pump event queue
  - Keep notification processing responsive

If the shell needs blocking reads, they should be bounded so BLE callbacks are never starved for long periods.

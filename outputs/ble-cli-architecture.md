# BLE CLI Architecture and Module Layout

## Purpose

This document proposes a clean internal structure for a BLE investigation CLI that supports discovery, pairing, GATT inspection, and live event handling for devices such as keyboards and gamepads.

The architecture is designed to keep transport logic, protocol decoding, and user interaction separate.

## High-Level Shape

The application should be split into five layers:

1. CLI shell and command parser
2. Session and context manager
3. BLE backend abstraction
4. GATT and protocol decoders
5. Presentation and output formatting

## Core Architectural Principles

- Keep the BLE backend replaceable per operating system.
- Keep protocol decoding independent from transport.
- Make context an explicit runtime state object.
- Treat live events as a stream, not as one-off command output.
- Preserve raw packets even when decoders exist.

## Suggested Top-Level Modules

```text
src/
  app/
    main
    shell
    prompt
    command-router
  core/
    context
    session
    device-model
    service-model
    characteristic-model
    descriptor-model
    event-bus
  backend/
    ble-backend
    windows-backend
    linux-backend
    macos-backend
  gatt/
    scanner
    connector
    pairing
    bonding
    service-discovery
    characteristic-ops
    descriptor-ops
  profiles/
    hid/
      report-map
      report-parser
      keyboard-decoder
      gamepad-decoder
    generic/
      notify-decoder
      hex-dump
  output/
    table-renderer
    json-renderer
    text-renderer
    log-writer
  scripts/
    batch-runner
    export
tests/
```

## Module Responsibilities

### `app/`

Handles user interaction.

- Read commands from the terminal
- Maintain the shell prompt
- Parse and route commands
- Support command history and tab completion if implemented later

### `core/`

Holds runtime state and object models.

- Current adapter, device, service, and characteristic
- Connection and pairing state
- Current output mode
- Event stream subscriptions
- Recently seen devices and cached metadata

### `backend/`

Provides OS-specific BLE operations.

- Scan for devices
- Connect and disconnect
- Read and write GATT values
- Subscribe to notifications
- Pair and bond if the platform supports it

The backend should expose one common interface so the rest of the app does not care whether the implementation is Windows, Linux, or macOS.

### `gatt/`

Handles BLE-specific protocol work.

- Service discovery
- Characteristic discovery
- Descriptor discovery
- Read and write operations
- Subscription management
- Pairing and security state transitions

### `profiles/`

Contains protocol-specific decoders.

#### HID

- Parse report maps
- Interpret keyboard input reports
- Interpret gamepad input reports
- Detect report IDs and usage pages

#### Generic

- Render raw bytes
- Convert to hex dumps
- Show notification timestamps

### `output/`

Formats results for the user.

- Tables for discovery and inspection
- JSON for scripting
- Text logs for live events
- Optional structured trace output for debugging

## Runtime Data Model

The shell should maintain a single session state object with these fields:

- active adapter
- active device
- active service
- active characteristic
- active descriptor
- connection state
- pairing state
- subscription list
- output mode
- decoder mode
- event log

The context object should be serializable so the tool can dump state or resume it later if desired.

## Backend Interface

The backend should expose a narrow API such as:

- `listAdapters()`
- `scanStart()`
- `scanStop()`
- `listDevices()`
- `connectDevice()`
- `disconnectDevice()`
- `pairDevice()`
- `readCharacteristic()`
- `writeCharacteristic()`
- `subscribeCharacteristic()`
- `unsubscribeCharacteristic()`
- `discoverServices()`
- `discoverCharacteristics()`
- `discoverDescriptors()`
- `readDescriptor()`

This keeps the shell and profile decoders independent from platform details.

## Event Flow

Live BLE events should move through the system like this:

1. Backend receives notification or indication
2. Core event bus normalizes the event
3. Profile decoder inspects the payload
4. Output layer renders raw and decoded forms
5. Shell optionally stores the event in history or a log file

This allows the same event to be shown raw, decoded, and logged without duplicating logic.

## HID-Specific Flow

For keyboards and gamepads, the HID path should be:

1. Discover HID service
2. Read report map if needed
3. Subscribe to input report characteristic
4. Parse report ID and payload
5. Decode through keyboard or gamepad profile
6. Emit both raw bytes and decoded state

## Suggested Package Boundaries

If implemented in a language with packages or namespaces, prefer these boundaries:

- `cli`
- `core`
- `ble`
- `gatt`
- `hid`
- `render`
- `tests`

Avoid mixing parser logic directly into device I/O handlers.

## Persistence Options

The MVP can stay in-memory, but the architecture should leave room for:

- device trust cache
- pairing/bonding records
- scan history
- session replay logs
- saved command scripts

## Test Strategy

Focus tests on the seams:

- command parsing
- context transitions
- backend interface behavior
- HID report decoding
- output formatting
- error translation

Where possible, build test fixtures from captured BLE packets so real keyboard and gamepad data can be replayed.


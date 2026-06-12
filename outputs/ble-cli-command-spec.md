# BLE CLI Command Spec

## Purpose

This document defines a context-driven command line interface for investigating, pairing, and receiving live events from BLE devices such as keyboards and gamepads.

The CLI is intended to feel somewhat like `diskpart` in that the prompt reflects the current target context, but the command model is adapted to BLE workflows.

## Design Goals

- Make discovery, pairing, inspection, and live monitoring available in one tool.
- Support both interactive use and scripted use.
- Keep generic BLE operations separate from profile-specific decoding.
- Preserve a visible context so the user always knows what object is selected.
- Make raw data available even when protocol decoding is incomplete.

## Core Concepts

The tool uses a nested context model:

- `adapter`: a local BLE radio/controller
- `device`: a discovered BLE peripheral
- `service`: a GATT service on the selected device
- `characteristic`: a GATT characteristic
- `descriptor`: metadata attached to a characteristic
- `session`: the live connection, pairing, and event stream state

The prompt should display the current path, for example:

```text
blectl /adapter/0 /device/AA:BB:CC:DD:EE:FF /service/1812 /char/2a4d>
```

## Global Commands

These commands work in any context.

- `help`
- `exit`
- `quit`
- `back`
- `root`
- `context`
- `status`
- `dump`
- `json on|off`
- `log on|off`
- `trace on|off`

## Adapter Commands

Adapter context controls local BLE hardware and scanning.

- `adapters`
  - List available adapters
- `select adapter <id>`
  - Select an adapter by numeric ID or name
- `adapter info`
  - Show adapter capabilities and current state
- `adapter power on|off`
  - Enable or disable the adapter
- `scan start`
  - Begin scanning for nearby devices
- `scan stop`
  - Stop the active scan
- `scan filter [name=<pattern>] [addr=<pattern>] [service=<uuid>]`
  - Apply scan filters
- `scan window <ms>`
  - Set scan window if supported
- `scan interval <ms>`
  - Set scan interval if supported
- `devices`
  - Show discovered devices

## Device Commands

Device context focuses on one discovered peripheral.

- `select device <address|index>`
  - Enter a device context
- `device info`
  - Show name, address, address type, RSSI, and known services
- `connect`
  - Connect to the selected device
- `disconnect`
  - Disconnect from the selected device
- `pair`
  - Initiate pairing
- `bond`
  - Create or confirm a bond if supported
- `forget`
  - Remove bond and stored security data
- `rssi`
  - Read current signal strength
- `services`
  - List services after connection
- `security`
  - Show security state and capabilities
- `mtu`
  - Show negotiated MTU if available
- `monitor`
  - Start live event monitoring for the device

## Service Commands

Service context narrows the selection to a GATT service.

- `select service <uuid|index>`
  - Select a service from the current device
- `service info`
  - Show UUID, handle range, and included services
- `chars`
  - List characteristics in the service
- `back`
  - Return to device context

## Characteristic Commands

Characteristic context is used for reading, writing, and subscriptions.

- `select char <uuid|index>`
  - Select a characteristic from the current service
- `char info`
  - Show UUID, properties, handle, and descriptors
- `read`
  - Read the current value
- `write <hex|text>`
  - Write data to the characteristic
- `subscribe on`
  - Enable notifications or indications
- `subscribe off`
  - Disable notifications or indications
- `notify`
  - Show live notifications for this characteristic
- `descriptors`
  - List descriptors attached to this characteristic
- `decode raw`
  - Show raw payloads
- `decode auto`
  - Apply the best available decoder

## Descriptor Commands

Descriptor context is mainly for inspection.

- `select desc <uuid|index>`
  - Select a descriptor
- `desc info`
  - Show descriptor metadata
- `read`
  - Read descriptor value

## HID Profile Commands

HID support should be first-class because keyboards and gamepads both use it.

- `hid map`
  - Read and display the Report Map
- `hid reports`
  - List discovered HID reports
- `hid report <id>`
  - Select a specific report
- `hid decode raw`
  - Print raw bytes only
- `hid decode keyboard`
  - Decode keyboard input reports
- `hid decode gamepad`
  - Decode gamepad input reports
- `hid boot on|off`
  - Switch boot protocol mode if supported
- `hid leds <mask>`
  - Send output report state for keyboard LEDs

## Generic Event Commands

These commands deal with live data regardless of profile.

- `events`
  - Show recent events
- `monitor`
  - Stream notifications and indications
- `pause`
  - Pause live output
- `resume`
  - Resume live output
- `clear`
  - Clear the screen or event buffer

## Command Style

The tool should support both interactive and scripted invocation.

Interactive examples:

```text
select adapter 0
scan start
devices
select device AA:BB:CC:DD:EE:FF
pair
connect
services
select service 1812
chars
select char 2a4d
subscribe on
monitor
```

Scripted examples:

```text
blectl device AA:BB:CC:DD:EE:FF connect
blectl device AA:BB:CC:DD:EE:FF service 1812 char 2a4d subscribe on
```

## Output Modes

The CLI should support multiple output styles:

- Human-readable tables for interactive use
- Raw text for logs
- JSON for automation
- Hex dumps for payload inspection

## Error Handling

The tool should always explain failures in plain terms.

- If pairing fails, show whether the issue was authentication, authorization, timeout, or transport.
- If a characteristic cannot be subscribed to, show whether notifications or indications are unsupported.
- If a profile decoder is unavailable, fall back to raw bytes rather than failing the command.

## Suggested Prompt Behavior

The prompt should reflect the current object chain.

Examples:

```text
blectl>
blectl /adapter/0>
blectl /adapter/0 /device/AA:BB:CC:DD:EE:FF>
blectl /adapter/0 /device/AA:BB:CC:DD:EE:FF /service/1812>
```

## Recommended Defaults

- Default to interactive shell mode when launched with no arguments.
- Default to `json off`.
- Default to raw event streaming when monitoring.
- Default to keeping the current context until changed or cleared.


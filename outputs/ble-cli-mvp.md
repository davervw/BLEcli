# BLE CLI MVP Feature List

## Goal

Build the smallest useful command line tool that can discover BLE devices, connect to one, pair when required, subscribe to notifications, and show raw and decoded event data for keyboards and gamepads.

## MVP Outcomes

The MVP should let a user:

- See available adapters
- Scan for nearby BLE peripherals
- Select a device
- Connect and pair
- Inspect GATT services and characteristics
- Subscribe to input notifications
- View raw bytes and a basic decoded view
- Keep a live event stream visible in the terminal

## Must-Have Features

### Discovery

- List adapters
- Start and stop scanning
- Show discovered devices with address, name, and RSSI
- Filter scan results by name or address substring

### Device Management

- Select one device as the current context
- Connect and disconnect
- Pair with the selected device
- Show whether the device is bonded
- Forget stored pairing data

### GATT Inspection

- List services
- List characteristics for a selected service
- List descriptors for a selected characteristic
- Read characteristic values
- Subscribe to notifications or indications

### Event Streaming

- Print raw notification payloads
- Timestamp each event
- Keep event output stable during long sessions
- Allow pausing and resuming the stream

### HID Support

- Read the HID Report Map
- Identify input report characteristics
- Decode keyboard reports at a basic level
- Decode gamepad reports at a basic level
- Fall back to raw bytes when decoding is incomplete

### Usability

- Maintain a context-aware prompt
- Support `back` and `root`
- Support `help`
- Support `status`
- Support both human-readable and JSON output

## Nice-to-Have But Not MVP

- Automatic report-map visualization
- Device-specific plugin packs
- Saved profiles for known devices
- Persistent pairing database across OS backends
- Replay mode for captured events
- Rich tab completion
- Script macros
- Advanced security diagnostics
- Cross-platform GUI front-end

## Explicit Non-Goals For MVP

- Full vendor-specific decoder coverage
- Support for every BLE profile
- GUI rendering
- Remote BLE proxying
- Deep packet capture of the entire BLE link layer
- Firmware flashing or device configuration beyond normal GATT operations

## Proposed MVP User Flow

```text
blectl
> adapters
> select adapter 0
> scan start
> devices
> select device AA:BB:CC:DD:EE:FF
> connect
> pair
> services
> select service 1812
> chars
> select char 2a4d
> subscribe on
> monitor
```

## Minimum Data Model

The MVP should track:

- active adapter
- active device
- active service
- active characteristic
- connection state
- pairing state
- scan results
- live notifications
- output mode

## Minimum Decoder Behavior

### Keyboard

- Show modifier bits
- Show pressed key codes
- Show report ID if present
- Show the raw hex payload

### Gamepad

- Show button bits
- Show axes if obvious from the report layout
- Show raw hex payload

If the report structure is not known, the tool should still display the bytes and let the user inspect them manually.

## MVP Implementation Order

1. Shell and command parser
2. Adapter scanning and device listing
3. Device connect and disconnect
4. GATT discovery
5. Characteristic subscribe and live event stream
6. Basic HID report display
7. Basic keyboard decoder
8. Basic gamepad decoder
9. JSON output mode

## Success Criteria

The MVP is successful if a user can connect to a real BLE keyboard or gamepad, subscribe to its input stream, and reliably see the arriving events in a usable form without needing to patch the tool each time.


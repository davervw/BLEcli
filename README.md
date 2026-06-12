# BLEcli - Bluetooth Low Energy CLI app including HID reports

Context based command line interface to find, select, connect, and monitor HID reports of BLE devices

* Game controller, Gamepad
* Mouse
* Trackpad
* Keyboard

Example sequence of commands

````
scan start
scan stop
devices
select device 0
connect
hid map
monitor
````

Output example:

````
00 00 04 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 05 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 06 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 07 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 08 00 00 00 00 00
00 00 08 09 00 00 00 00
````

Notes

* Targeting ESP32
* Built using platformio environment
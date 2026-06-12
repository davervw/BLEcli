#pragma once

#include <Arduino.h>

class HidProfile {
public:
  static String formatHex(const uint8_t* data, size_t length);
  static String decodeKeyboard(const uint8_t* data, size_t length);
  static String decodeGamepad(const uint8_t* data, size_t length);
};


#include "HidProfile.h"

String HidProfile::formatHex(const uint8_t* data, size_t length) {
  String out;
  for (size_t i = 0; i < length; ++i) {
    if (i > 0) {
      out += ' ';
    }
    if (data[i] < 0x10) {
      out += '0';
    }
    out += String(data[i], HEX);
  }
  out.toUpperCase();
  return out;
}

String HidProfile::decodeKeyboard(const uint8_t* data, size_t length) {
  if (length < 2) {
    return String();
  }

  String out = "kbd mods=0x";
  if (data[0] < 0x10) {
    out += '0';
  }
  out += String(data[0], HEX);
  out += " keys=";

  bool any = false;
  for (size_t i = 2; i < length && i < 8; ++i) {
    if (data[i] == 0) {
      continue;
    }
    if (any) {
      out += ',';
    }
    if (data[i] < 0x10) {
      out += '0';
    }
    out += String(data[i], HEX);
    any = true;
  }

  if (!any) {
    out += "<none>";
  }

  out.toUpperCase();
  return out;
}

String HidProfile::decodeGamepad(const uint8_t* data, size_t length) {
  if (length == 0) {
    return String();
  }

  String out = "pad len=";
  out += String(length);
  out += " raw=";
  out += formatHex(data, length);
  return out;
}


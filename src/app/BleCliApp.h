#pragma once

#include <Arduino.h>
#include <vector>

#include "backend/esp32/Esp32BleBackend.h"
#include "core/BleSession.h"

class BleCliApp {
public:
  void begin();
  void loop();

private:
  void pumpSerial();
  void handleLine(const String& line);
  void printHelp() const;
  void printPrompt() const;
  void printDevices() const;
  void printStatus() const;
  void selectDeviceArg(const String& arg);
  void rebootDevice();

  static std::vector<String> splitTokens(const String& line);

  BleSession session_;
  Esp32BleBackend backend_;
  String inputBuffer_;
};

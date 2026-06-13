#include "BleCliApp.h"

#include <M5Unified.h>

void BleCliApp::begin() {
  session_.reset();
  backend_.begin();

  Serial.println();
  Serial.println("BLECLI ready.");
  Serial.println("Type help for commands.");
  printPrompt();
}

void BleCliApp::loop() {
  session_.setConnected(backend_.isConnected());
  pumpSerial();
}

void BleCliApp::pumpSerial() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }

    if (ch == '\b') {
      if (inputBuffer_.length() > 0) {
        inputBuffer_.remove(inputBuffer_.length() - 1);
        Serial.print(" \b");
      } else
        Serial.print(" ");
      continue;
    }

    if (ch == '\n') {
      String line = inputBuffer_;
      inputBuffer_.clear();
      handleLine(line);
      printPrompt();
      continue;
    }

    inputBuffer_ += ch;
  }
}

std::vector<String> BleCliApp::splitTokens(const String& line) {
  std::vector<String> tokens;
  String current;
  for (size_t i = 0; i < line.length(); ++i) {
    const char ch = line[i];
    if (ch == ' ' || ch == '\t') {
      if (current.length() > 0) {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }
    current += ch;
  }
  if (current.length() > 0) {
    tokens.push_back(current);
  }
  return tokens;
}

void BleCliApp::handleLine(const String& line) {
  String trimmed = line;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    return;
  }

  trimmed.toLowerCase();
  const auto tokens = splitTokens(trimmed);
  if (tokens.empty()) {
    return;
  }

  const String& cmd = tokens[0];

  if (cmd == "help") {
    printHelp();
    return;
  }

  if (cmd == "scan") {
    if (tokens.size() > 1 && tokens[1] == "start") {
      session_.setScanning(true);
      backend_.clearDevices();
      backend_.startScan();
      Serial.println("Scanning started.");
      return;
    }
    if (tokens.size() > 1 && tokens[1] == "stop") {
      session_.setScanning(false);
      backend_.stopScan();
      Serial.println("Scanning stopped.");
      return;
    }
  }

  if (cmd == "devices") {
    printDevices();
    return;
  }

  if (cmd == "select" && tokens.size() >= 3 && tokens[1] == "device") {
    selectDeviceArg(tokens[2]);
    return;
  }

  if (cmd == "connect") {
    if (!backend_.connectSelected()) {
      Serial.println("Connect failed.");
    } else {
      session_.setConnected(true);
      const BleDeviceRecord* d = backend_.selectedDevice();
      if (d != nullptr) {
        session_.selectDevice(d->index, d->address, d->name);
      }
    }
    return;
  }

  if (cmd == "disconnect") {
    backend_.disconnect();
    session_.setConnected(false);
    Serial.println("Disconnected.");
    return;
  }

  if (cmd == "pair") {
    Serial.println("Pairing is currently handled by the NimBLE security handshake during connect.");
    Serial.println("If the device needs passkey or confirmation, we can wire that next.");
    return;
  }

  if (cmd == "services") {
    Serial.println(backend_.serviceSummary());
    return;
  }

  if (cmd == "hid" && tokens.size() >= 2 && tokens[1] == "map") {
    Serial.println(backend_.dumpHidReportMap());
    return;
  }

  if (cmd == "hid" && tokens.size() >= 2 && tokens[1] == "chars") {
    Serial.print(backend_.dumpHidCharacteristics());
    return;
  }

  if (cmd == "hid" && tokens.size() >= 2 && tokens[1] == "reports") {
    Serial.print(backend_.dumpHidReports());
    return;
  }

  if (cmd == "monitor") {
    if (!backend_.subscribeHidInput()) {
      Serial.println("Subscribe failed.");
    } else {
      Serial.println("Monitoring HID input.");
    }
    return;
  }

  if (cmd == "reboot" || cmd == "restart") {
    rebootDevice();
    return;
  }

  if (cmd == "status") {
    printStatus();
    return;
  }

  if (cmd == "back") {
    session_.clearCharacteristic();
    session_.clearService();
    backend_.clearSelection();
    Serial.println("Moved up one level.");
    return;
  }

  if (cmd == "root") {
    session_.clearCharacteristic();
    session_.clearService();
    session_.clearDevice();
    backend_.clearSelection();
    Serial.println("Returned to root.");
    return;
  }

  Serial.println("Unknown command. Type help.");
}

void BleCliApp::printHelp() const {
  Serial.println("Commands:");
  Serial.println("  help");
  Serial.println("  scan start|stop");
  Serial.println("  devices");
  Serial.println("  select device <index|address>");
  Serial.println("  connect");
  Serial.println("  disconnect");
  Serial.println("  pair");
  Serial.println("  services");
  Serial.println("  hid map");
  Serial.println("  hid chars");
  Serial.println("  hid reports");
  Serial.println("  monitor");
  Serial.println("  status");
  Serial.println("  reboot");
  Serial.println("  back");
  Serial.println("  root");
}

void BleCliApp::printPrompt() const {
  Serial.print(session_.prompt());
}

void BleCliApp::printDevices() const {
  const auto& list = backend_.devices();
  if (list.empty()) {
    Serial.println("No devices recorded. Run scan start first.");
    return;
  }

  for (const auto& d : list) {
    Serial.print(d.index);
    Serial.print(": ");
    Serial.print(d.address);
    Serial.print(" ");
    Serial.print(d.name);
    Serial.print(" RSSI=");
    Serial.print(d.rssi);
    Serial.print(" ");
    Serial.println(d.connectable ? "conn" : "no-conn");
  }
}

void BleCliApp::printStatus() const {
  Serial.println(session_.statusLine());
  Serial.println(backend_.serviceSummary());
}

void BleCliApp::selectDeviceArg(const String& arg) {
  bool numeric = true;
  for (size_t i = 0; i < arg.length(); ++i) {
    if (!isDigit(arg[i])) {
      numeric = false;
      break;
    }
  }

  bool ok = false;
  if (numeric) {
    ok = backend_.selectDeviceByIndex(arg.toInt());
  } else {
    ok = backend_.selectDeviceByAddress(arg);
  }

  if (!ok) {
    Serial.println("Device not found.");
    return;
  }

  const BleDeviceRecord* d = backend_.selectedDevice();
  if (d != nullptr) {
    session_.selectDevice(d->index, d->address, d->name);
    Serial.print("Selected device ");
    Serial.print(d->index);
    Serial.print(": ");
    Serial.println(d->address);
  }
}

void BleCliApp::rebootDevice() {
  Serial.println("Rebooting...");
  M5.Display.fillScreen(TFT_BLACK);
  delay(1500);
  ESP.restart();
}

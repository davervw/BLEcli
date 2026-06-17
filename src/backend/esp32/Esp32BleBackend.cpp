#include "Esp32BleBackend.h"

#include "profiles/hid/HidProfile.h"

namespace {
Esp32BleBackend* g_backend = nullptr;

void notifyCallback(NimBLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
  if (g_backend != nullptr) {
    g_backend->handleNotify(characteristic, data, length, isNotify);
  }
}
} // namespace

Esp32BleBackend::Esp32BleBackend()
  : scanCallbacks_(this), clientCallbacks_(this) {
  g_backend = this;
}

bool Esp32BleBackend::begin() {
  NimBLEDevice::init("BLECLI");
  NimBLEDevice::setPower(3);
  NimBLEDevice::setSecurityAuth(true, false, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(&scanCallbacks_, false);
  scan->setInterval(100);
  scan->setWindow(100);
  scan->setActiveScan(true);
  return true;
}

bool Esp32BleBackend::startScan() {
  scanning_ = true;
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(&scanCallbacks_, false);
  scan->start(scanTimeMs_, false, true);
  return true;
}

void Esp32BleBackend::stopScan() {
  scanning_ = false;
  NimBLEDevice::getScan()->stop();
}

const std::vector<BleDeviceRecord>& Esp32BleBackend::devices() const {
  return devices_;
}

void Esp32BleBackend::clearDevices() {
  devices_.clear();
  selectedIndex_ = -1;
}

void Esp32BleBackend::clearSelection() {
  selectedIndex_ = -1;
}

bool Esp32BleBackend::selectDeviceByIndex(int index) {
  if (index < 0 || index >= static_cast<int>(devices_.size())) {
    return false;
  }
  selectedIndex_ = index;
  return true;
}

bool Esp32BleBackend::selectDeviceByAddress(const String& address) {
  for (size_t i = 0; i < devices_.size(); ++i) {
    if (devices_[i].address.equalsIgnoreCase(address)) {
      selectedIndex_ = static_cast<int>(i);
      return true;
    }
  }
  return false;
}

bool Esp32BleBackend::connectSelected() {
  const BleDeviceRecord* selected = selectedDevice();
  if (selected == nullptr) {
    Serial.println("No device selected.");
    return false;
  }

  stopScan();

  NimBLEAddress address(selected->address.c_str(), selected->addressType);

  if (NimBLEDevice::getCreatedClientCount()) {
    client_ = NimBLEDevice::getClientByPeerAddress(address);
    if (client_ != nullptr && client_->isConnected()) {
      Serial.println("Already connected.");
      return true;
    }
    if (client_ == nullptr) {
      client_ = NimBLEDevice::getDisconnectedClient();
    }
  }

  if (client_ == nullptr) {
    client_ = NimBLEDevice::createClient();
    if (client_ == nullptr) {
      Serial.println("Failed to create client.");
      return false;
    }
    client_->setClientCallbacks(&clientCallbacks_, false);
    client_->setConnectionParams(12, 12, 0, 150);
    client_->setConnectTimeout(5000);
  }

  if (!client_->connect(address, false)) {
    Serial.println("Connect failed.");
    return false;
  }

  Serial.print("Connected to ");
  Serial.println(selected->address);
  return true;
}

void Esp32BleBackend::disconnect() {
  if (client_ != nullptr && client_->isConnected()) {
    clearMonitorSubscriptions();
    client_->disconnect();
  }
}

bool Esp32BleBackend::isConnected() const {
  return client_ != nullptr && client_->isConnected();
}

bool Esp32BleBackend::subscribeHidInput() {
  if (!isConnected()) {
    Serial.println("Not connected.");
    return false;
  }

  clearMonitorSubscriptions();
  lastNotifyKey_.clear();
  lastNotifyPayload_.clear();
  lastNotifyMs_ = 0;

  NimBLERemoteService* hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid == nullptr) {
    Serial.println("HID service not found.");
    return false;
  }

  bool subscribed = false;
  const auto& chars = hid->getCharacteristics(true);
  for (auto* chr : chars) {
    if (chr == nullptr) {
      continue;
    }

    uint8_t reportId = 0;
    const bool isBootKeyboard = chr->getUUID().equals(NimBLEUUID((uint16_t)0x2A22));
    const bool isInputReport = isHidInputReportCharacteristic(chr, &reportId);
    if (!isBootKeyboard && !isInputReport) {
      continue;
    }

    Serial.print("Inspecting ");
    Serial.print(chr->getUUID().toString().c_str());
    Serial.print(" notify=");
    Serial.print(chr->canNotify() ? "y" : "n");
    Serial.print(" indicate=");
    Serial.println(chr->canIndicate() ? "y" : "n");

    if (chr->canNotify()) {
      if (chr->subscribe(true, notifyCallback)) {
        subscribed = true;
        subscribedHandles_.push_back(chr->getHandle());
        Serial.print("Subscribed notify ");
        Serial.println(chr->getUUID().toString().c_str());
      } else {
        Serial.print("Failed subscribe notify ");
        Serial.println(chr->getUUID().toString().c_str());
      }
    } else if (chr->canIndicate()) {
      if (chr->subscribe(false, notifyCallback)) {
        subscribed = true;
        subscribedHandles_.push_back(chr->getHandle());
        Serial.print("Subscribed indicate ");
        Serial.println(chr->getUUID().toString().c_str());
      } else {
        Serial.print("Failed subscribe indicate ");
        Serial.println(chr->getUUID().toString().c_str());
      }
    }
  }

  if (!subscribed) {
    Serial.println("No HID characteristics were subscribable.");
  }
  return subscribed;
}

String Esp32BleBackend::dumpHidReports() {
  if (!isConnected()) {
    return "Not connected.";
  }

  NimBLERemoteService* hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid == nullptr) {
    return "HID service not found.";
  }

  String out;
  const auto& chars = hid->getCharacteristics(true);
  for (auto* chr : chars) {
    if (chr == nullptr) {
      continue;
    }

    uint8_t reportId = 0;
    bool isInput = isHidInputReportCharacteristic(chr, &reportId);
    const bool isBootKeyboard = chr->getUUID().equals(NimBLEUUID((uint16_t)0x2A22));

    out += "handle=";
    out += String(chr->getHandle());
    out += " uuid=";
    out += chr->getUUID().toString().c_str();
    out += " notify=";
    out += chr->canNotify() ? "y" : "n";
    out += " indicate=";
    out += chr->canIndicate() ? "y" : "n";
    if (isBootKeyboard) {
      out += " report=boot-kbd";
    } else if (isInput) {
      out += " reportId=";
      out += String(reportId);
      out += " type=";
      out += reportId == 1 ? "keyboard" : reportId == 3 ? "trackpad" : "input";
    } else {
      out += " type=other";
    }
    out += "\n";
  }

  return out;
}

String Esp32BleBackend::dumpHidCharacteristics() {
  if (!isConnected()) {
    return "Not connected.";
  }

  NimBLERemoteService* hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid == nullptr) {
    return "HID service not found.";
  }

  String out;
  const auto& chars = hid->getCharacteristics(true);
  for (auto* chr : chars) {
    if (chr == nullptr) {
      continue;
    }
    out += chr->getUUID().toString().c_str();
    out += " notify=";
    out += chr->canNotify() ? "y" : "n";
    out += " indicate=";
    out += chr->canIndicate() ? "y" : "n";
    out += "\n";
  }
  return out;
}

String Esp32BleBackend::dumpHidReportMap() {
  NimBLERemoteCharacteristic* mapChar = nullptr;
  NimBLERemoteService* hid = nullptr;

  if (!isConnected()) {
    return "Not connected.";
  }

  hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid == nullptr) {
    return "HID service not found.";
  }

  mapChar = hid->getCharacteristic(NimBLEUUID((uint16_t)0x2A4B));
  if (mapChar == nullptr) {
    return "HID Report Map not found.";
  }

  std::string value = mapChar->readValue();
  if (value.empty()) {
    return "HID Report Map is empty.";
  }

  return HidProfile::formatHex(reinterpret_cast<const uint8_t*>(value.data()), value.size());
}

String Esp32BleBackend::serviceSummary() const {
  if (!isConnected()) {
    return "Not connected.";
  }

  String out = "conn=on ";
  if (selectedDevice() != nullptr) {
    out += selectedDevice()->address;
  } else {
    out += "<unknown>";
  }

  NimBLERemoteService* hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid != nullptr) {
    out += " HID=on";
  } else {
    out += " HID=off";
  }
  return out;
}

const BleDeviceRecord* Esp32BleBackend::selectedDevice() const {
  if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(devices_.size())) {
    return nullptr;
  }
  return &devices_[selectedIndex_];
}

void Esp32BleBackend::handleScanResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  if (advertisedDevice == nullptr) {
    return;
  }

  bool isAdvertisingHID = advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID((uint16_t)0x1812));

  String address = advertisedDevice->getAddress().toString().c_str();
  uint8_t addressType = advertisedDevice->getAddressType();
  String name = advertisedDevice->haveName() ? advertisedDevice->getName().c_str() : String("<unnamed>");
  int rssi = advertisedDevice->getRSSI();
  bool connectable = advertisedDevice->isConnectable();

  for (size_t i = 0; i < devices_.size(); ++i) {
    if (devices_[i].address.equalsIgnoreCase(address)) {
      devices_[i].name = name;
      devices_[i].rssi = rssi;
      devices_[i].connectable = connectable;
      devices_[i].addressType = addressType;
      devices_[i].index = static_cast<int>(i);
      devices_[i].appearance = advertisedDevice->haveAppearance() ? advertisedDevice->getAppearance() : 0;
      devices_[i].hid = isAdvertisingHID;
      return;
    }
  }

  BleDeviceRecord record;
  record.index = static_cast<int>(devices_.size());
  record.address = address;
  record.addressType = addressType;
  record.name = name;
  record.rssi = rssi;
  record.connectable = connectable;
  devices_.push_back(record);

  Serial.print("Found ");
  Serial.print(record.index);
  Serial.print(": ");
  Serial.print(record.address);
  Serial.print(" ");
  Serial.print(record.name);
  Serial.print(" RSSI=");
  Serial.println(record.rssi);
}

void Esp32BleBackend::handleScanEnd(int reason) {
  Serial.print("Scan ended, reason=");
  Serial.println(reason);
  if (scanning_ && reconnectScan_) {
    NimBLEDevice::getScan()->start(scanTimeMs_, false, true);
  }
}

void Esp32BleBackend::handleConnect() {
  Serial.println("Connected.");
}

void Esp32BleBackend::handleDisconnect(int reason) {
  Serial.print("Disconnected, reason=");
  Serial.println(reason);
  clearMonitorSubscriptions();
  if (scanning_ && reconnectScan_) {
    NimBLEDevice::getScan()->start(scanTimeMs_, false, true);
  }
}

void Esp32BleBackend::handleNotify(NimBLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
  if (characteristic == nullptr || data == nullptr || length == 0) {
    return;
  }

  // Prevent concurrent re-entry from other notify callbacks.
  if (notifyLock_.test_and_set(std::memory_order_acquire)) {
    // Another handleNotify is in progress; skip this one.
    return;
  }

  // RAII guard to ensure the lock is released on exit.
  struct NotifyLockGuard {
    std::atomic_flag& flag;
    NotifyLockGuard(std::atomic_flag& f) : flag(f) {}
    ~NotifyLockGuard() { flag.clear(std::memory_order_release); }
  } guard(notifyLock_);

  String raw = HidProfile::formatHex(data, length);
  String key = String(characteristic->getHandle());
  key += ":";
  key += characteristic->getUUID().toString().c_str();
  uint32_t now = millis();
  if (!shouldPrintNotification(key, raw, now)) {
    return;
  }

  Serial.print(raw);
  Serial.println();
}

bool Esp32BleBackend::isHidInputReportCharacteristic(NimBLERemoteCharacteristic* characteristic, uint8_t* reportIdOut) const {
  if (characteristic == nullptr) {
    return false;
  }

  if (!characteristic->getUUID().equals(NimBLEUUID((uint16_t)0x2A4D))) {
    return false;
  }

  const auto& descriptors = characteristic->getDescriptors(true);
  for (auto* desc : descriptors) {
    if (desc == nullptr) {
      continue;
    }
    if (!desc->getUUID().equals(NimBLEUUID((uint16_t)0x2908))) {
      continue;
    }

    std::string value = desc->readValue();
    if (value.size() < 2) {
      continue;
    }

    const uint8_t reportId = static_cast<uint8_t>(value[0]);
    const uint8_t reportType = static_cast<uint8_t>(value[1]);
    if (reportIdOut != nullptr) {
      *reportIdOut = reportId;
    }
    return reportType == 1;
  }

  return false;
}

NimBLERemoteCharacteristic* Esp32BleBackend::findHidCharacteristicByHandle(uint16_t handle) const {
  if (!isConnected()) {
    return nullptr;
  }

  NimBLERemoteService* hid = client_->getService(NimBLEUUID((uint16_t)0x1812));
  if (hid == nullptr) {
    return nullptr;
  }

  const auto& chars = hid->getCharacteristics(true);
  for (auto* chr : chars) {
    if (chr != nullptr && chr->getHandle() == handle) {
      return chr;
    }
  }

  return nullptr;
}

void Esp32BleBackend::clearMonitorSubscriptions() {
  if (!isConnected()) {
    subscribedHandles_.clear();
    return;
  }

  for (uint16_t handle : subscribedHandles_) {
    NimBLERemoteCharacteristic* chr = findHidCharacteristicByHandle(handle);
    if (chr != nullptr) {
      chr->unsubscribe(true);
    }
  }

  subscribedHandles_.clear();
}

String Esp32BleBackend::reportTypeName(uint8_t reportType) const {
  switch (reportType) {
    case 1:
      return "input";
    case 2:
      return "output";
    case 3:
      return "feature";
    default:
      return "unknown";
  }
}

bool Esp32BleBackend::shouldPrintNotification(const String& key, const String& payload, uint32_t nowMs) {
  const bool sameAsLast = key == lastNotifyKey_ && payload == lastNotifyPayload_;
  if (!sameAsLast) {
    lastNotifyKey_ = key;
    lastNotifyPayload_ = payload;
    lastNotifyMs_ = nowMs;
    return true;
  }

  if (nowMs - lastNotifyMs_ >= 1000UL) {
    lastNotifyMs_ = nowMs;
    return true;
  }

  return false;
}

void Esp32BleBackend::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  if (owner_ != nullptr) {
    owner_->handleScanResult(advertisedDevice);
  }
}

void Esp32BleBackend::ScanCallbacks::onScanEnd(const NimBLEScanResults&, int reason) {
  if (owner_ != nullptr) {
    owner_->handleScanEnd(reason);
  }
}

void Esp32BleBackend::ClientCallbacks::onConnect(NimBLEClient*) {
  if (owner_ != nullptr) {
    owner_->handleConnect();
  }
}

void Esp32BleBackend::ClientCallbacks::onDisconnect(NimBLEClient*, int reason) {
  if (owner_ != nullptr) {
    owner_->handleDisconnect(reason);
  }
}

void Esp32BleBackend::ClientCallbacks::onPassKeyEntry(NimBLEConnInfo& connInfo) {
  Serial.println("Passkey requested; injecting default test value.");
  NimBLEDevice::injectPassKey(connInfo, 123456);
}

void Esp32BleBackend::ClientCallbacks::onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t passkey) {
  Serial.print("Confirming passkey ");
  Serial.println(passkey);
  NimBLEDevice::injectConfirmPasskey(connInfo, true);
}

void Esp32BleBackend::ClientCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (!connInfo.isEncrypted()) {
    Serial.println("Authentication complete, but link is not encrypted.");
    NimBLEClient* client = NimBLEDevice::getClientByHandle(connInfo.getConnHandle());
    if (client != nullptr) {
      client->disconnect();
    }
  } else {
    Serial.println("Authentication complete.");
  }
}

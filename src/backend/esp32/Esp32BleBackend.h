#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <vector>

struct BleDeviceRecord {
  int index = -1;
  String address;
  uint8_t addressType = 0;
  String name;
  int rssi = 0;
  bool connectable = false;
  bool hid = false;
  uint16_t appearance = 0;
};

class Esp32BleBackend {
public:
  Esp32BleBackend();

  bool begin();
  bool startScan();
  void stopScan();

  const std::vector<BleDeviceRecord>& devices() const;
  void clearDevices();
  void clearSelection();

  bool selectDeviceByIndex(int index);
  bool selectDeviceByAddress(const String& address);

  bool connectSelected();
  void disconnect();
  bool isConnected() const;

  bool subscribeHidInput();
  String dumpHidReports();
  String dumpHidCharacteristics();
  String dumpHidReportMap();
  String serviceSummary() const;

  const BleDeviceRecord* selectedDevice() const;
  void handleNotify(NimBLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify);

private:
  class ScanCallbacks : public NimBLEScanCallbacks {
  public:
    explicit ScanCallbacks(Esp32BleBackend* owner) : owner_(owner) {}
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
  private:
    Esp32BleBackend* owner_;
  };

  class ClientCallbacks : public NimBLEClientCallbacks {
  public:
    explicit ClientCallbacks(Esp32BleBackend* owner) : owner_(owner) {}
    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;
    void onPassKeyEntry(NimBLEConnInfo& connInfo) override;
    void onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t passkey) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
  private:
    Esp32BleBackend* owner_;
  };

  void handleScanResult(const NimBLEAdvertisedDevice* advertisedDevice);
  void handleScanEnd(int reason);
  void handleConnect();
  void handleDisconnect(int reason);
  NimBLERemoteCharacteristic* findHidInputCharacteristic();
  NimBLERemoteCharacteristic* findHidCharacteristicByHandle(uint16_t handle) const;
  void clearMonitorSubscriptions();
  bool isHidInputReportCharacteristic(NimBLERemoteCharacteristic* characteristic, uint8_t* reportIdOut = nullptr) const;
  String reportTypeName(uint8_t reportType) const;
  bool shouldPrintNotification(const String& key, const String& payload, uint32_t nowMs);

  std::vector<BleDeviceRecord> devices_;
  int selectedIndex_ = -1;
  NimBLEClient* client_ = nullptr;
  bool scanning_ = false;
  bool reconnectScan_ = true;
  uint32_t scanTimeMs_ = 5000;
  String lastNotifyKey_;
  String lastNotifyPayload_;
  uint32_t lastNotifyMs_ = 0;
  std::vector<uint16_t> subscribedHandles_;

  ScanCallbacks scanCallbacks_;
  ClientCallbacks clientCallbacks_;
};

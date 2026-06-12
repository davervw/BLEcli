#pragma once

#include <Arduino.h>

class BleSession {
public:
  void reset();

  void setScanning(bool value);
  void setConnected(bool value);

  void selectDevice(int index, const String& address, const String& name);
  void clearDevice();

  bool hasDevice() const;
  int selectedDeviceIndex() const;
  const String& selectedDeviceAddress() const;
  const String& selectedDeviceName() const;

  void selectService(const String& uuid);
  void clearService();
  bool hasService() const;
  const String& selectedServiceUuid() const;

  void selectCharacteristic(const String& uuid);
  void clearCharacteristic();
  bool hasCharacteristic() const;
  const String& selectedCharacteristicUuid() const;

  String prompt() const;
  String statusLine() const;

private:
  bool scanning_ = false;
  bool connected_ = false;
  int selectedDeviceIndex_ = -1;
  String selectedDeviceAddress_;
  String selectedDeviceName_;
  String selectedServiceUuid_;
  String selectedCharacteristicUuid_;
};


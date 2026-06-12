#include "BleSession.h"

void BleSession::reset() {
  scanning_ = false;
  connected_ = false;
  selectedDeviceIndex_ = -1;
  selectedDeviceAddress_.clear();
  selectedDeviceName_.clear();
  selectedServiceUuid_.clear();
  selectedCharacteristicUuid_.clear();
}

void BleSession::setScanning(bool value) {
  scanning_ = value;
}

void BleSession::setConnected(bool value) {
  connected_ = value;
}

void BleSession::selectDevice(int index, const String& address, const String& name) {
  selectedDeviceIndex_ = index;
  selectedDeviceAddress_ = address;
  selectedDeviceName_ = name;
  clearService();
  clearCharacteristic();
}

void BleSession::clearDevice() {
  selectedDeviceIndex_ = -1;
  selectedDeviceAddress_.clear();
  selectedDeviceName_.clear();
  clearService();
  clearCharacteristic();
}

bool BleSession::hasDevice() const {
  return selectedDeviceIndex_ >= 0;
}

int BleSession::selectedDeviceIndex() const {
  return selectedDeviceIndex_;
}

const String& BleSession::selectedDeviceAddress() const {
  return selectedDeviceAddress_;
}

const String& BleSession::selectedDeviceName() const {
  return selectedDeviceName_;
}

void BleSession::selectService(const String& uuid) {
  selectedServiceUuid_ = uuid;
  clearCharacteristic();
}

void BleSession::clearService() {
  selectedServiceUuid_.clear();
}

bool BleSession::hasService() const {
  return selectedServiceUuid_.length() > 0;
}

const String& BleSession::selectedServiceUuid() const {
  return selectedServiceUuid_;
}

void BleSession::selectCharacteristic(const String& uuid) {
  selectedCharacteristicUuid_ = uuid;
}

void BleSession::clearCharacteristic() {
  selectedCharacteristicUuid_.clear();
}

bool BleSession::hasCharacteristic() const {
  return selectedCharacteristicUuid_.length() > 0;
}

const String& BleSession::selectedCharacteristicUuid() const {
  return selectedCharacteristicUuid_;
}

String BleSession::prompt() const {
  String p = "BLECLI";
  if (hasDevice()) {
    p += " /device/";
    p += selectedDeviceAddress_;
  }
  if (hasService()) {
    p += " /service/";
    p += selectedServiceUuid_;
  }
  if (hasCharacteristic()) {
    p += " /char/";
    p += selectedCharacteristicUuid_;
  }
  p += "> ";
  return p;
}

String BleSession::statusLine() const {
  String s;
  s += scanning_ ? "scan=on" : "scan=off";
  s += ", ";
  s += connected_ ? "conn=on" : "conn=off";
  s += ", ";
  s += "device=";
  s += hasDevice() ? selectedDeviceAddress_ : String("<none>");
  return s;
}

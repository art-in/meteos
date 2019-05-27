#include <Arduino.h>

#include "radio-bt.h"
#include "utils.h"

void RadioBT::start_advertising() {
  log_ln("bt: start advertising...", true);
  auto before_ms = millis();

  BLEDevice::init("Meteos Sensor");
  BLEServer *server_ptr = BLEDevice::createServer();

  server_ptr->setCallbacks(new ServerCallbacks());

  service_ptr = server_ptr->createService(SERVICE_UUID);
  BLECharacteristic *ssidCharacteristic = service_ptr->createCharacteristic(
      SSID_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *passCharacteristic = service_ptr->createCharacteristic(
      PASS_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  ssidCharacteristic->setValue(config.wifi_ssid);
  passCharacteristic->setValue(config.wifi_pass);

  ssidCharacteristic->setCallbacks(new SSIDCharacteristicCallbacks(config));
  passCharacteristic->setCallbacks(new PassCharacteristicCallbacks(config));

  service_ptr->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  log_ln(
      "bt: start advertising...done in " + String(millis() - before_ms) + "ms",
      true);
}

void ServerCallbacks::onConnect(BLEServer *server_ptr) {
  log_ln("bt: client connected.", true);
};

void ServerCallbacks::onDisconnect(BLEServer *server_ptr) {
  log_ln("bt: client disconnected.", true);
}

void SSIDCharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic) {
  log("bt: ssid read: ");
  log_ln(pCharacteristic->getValue().c_str());
}

void SSIDCharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  config.set_wifi_ssid(pCharacteristic->getValue());
  log_ln("bt: ssid written");
}

void PassCharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic) {
  log("bt: pass read: ");
  log_ln(pCharacteristic->getValue().c_str());
}

void PassCharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  config.set_wifi_pass(pCharacteristic->getValue());
  log_ln("bt: pass written");
}
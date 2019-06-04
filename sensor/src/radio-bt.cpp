#include <Arduino.h>

#include "radio-bt.h"
#include "utils.h"

constexpr auto SERVICE_UUID = "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6";
constexpr auto CHAR_UUID_SSID = "d1fe36b7-3223-4927-ada5-422f2b5afcf9";
constexpr auto CHAR_UUID_PASS = "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9";

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server_ptr) {
    METEOS_LOG_LN("bt: client connected.");
  }
  void onDisconnect(BLEServer *server_ptr) {
    METEOS_LOG_LN("bt: client disconnected.");
  };
};

class SSIDCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  SSIDCharacteristicCallbacks(Config &cfg) : config{cfg} {}

 private:
  Config &config;
  void onRead(BLECharacteristic *char_ptr) {
    METEOS_LOG("bt: ssid read: ");
    METEOS_LOG_LN(char_ptr->getValue().c_str());
  };
  void onWrite(BLECharacteristic *char_ptr) {
    config.wifi_ssid(char_ptr->getValue());
    METEOS_LOG_LN("bt: ssid written");
  };
};

class PassCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  PassCharacteristicCallbacks(Config &cfg) : config{cfg} {}

 private:
  Config &config;
  void onRead(BLECharacteristic *char_ptr) {
    METEOS_LOG("bt: pass read: ");
    METEOS_LOG_LN(char_ptr->getValue().c_str());
  };
  void onWrite(BLECharacteristic *char_ptr) {
    config.wifi_pass(char_ptr->getValue());
    METEOS_LOG_LN("bt: pass written");
  };
};

void RadioBT::init() {
  METEOS_SCOPED_LOGGER("bt: init");

  // init GATT server, service and characteristics
  BLEDevice::init("Meteos Sensor");
  BLEServer *server_ptr = BLEDevice::createServer();

  server_ptr->setCallbacks(new ServerCallbacks());

  service_ptr = server_ptr->createService(SERVICE_UUID);
  BLECharacteristic *char_ssid = service_ptr->createCharacteristic(
      CHAR_UUID_SSID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *char_pass = service_ptr->createCharacteristic(
      CHAR_UUID_PASS,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  char_ssid->setValue(config.wifi_ssid());
  char_pass->setValue(config.wifi_pass());

  char_ssid->setCallbacks(new SSIDCharacteristicCallbacks(config));
  char_pass->setCallbacks(new PassCharacteristicCallbacks(config));

  service_ptr->start();

  // init advertising
  advertising_ptr = BLEDevice::getAdvertising();
  advertising_ptr->addServiceUUID(SERVICE_UUID);
  advertising_ptr->setScanResponse(true);

  // functions that help with iPhone connections issue
  advertising_ptr->setMinPreferred(0x06);
  advertising_ptr->setMinPreferred(0x12);
}

void RadioBT::start_advertising() {
  METEOS_SCOPED_LOGGER("bt: start advertising");

  advertising_ptr->start();
}

void RadioBT::stop_advertising() {
  METEOS_SCOPED_LOGGER("bt: stop advertising");

  advertising_ptr->stop();
}
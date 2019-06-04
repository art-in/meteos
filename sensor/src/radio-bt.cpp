#include <Arduino.h>
#include <functional>
#include <string>

#include "radio-bt.h"
#include "utils.h"

constexpr auto SERVICE_UUID = "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6";
constexpr auto CHAR_UUID_WIFI_SSID = "d1fe36b7-3223-4927-ada5-422f2b5afcf9";
constexpr auto CHAR_UUID_WIFI_PASS = "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9";
constexpr auto CHAR_UUID_BACKEND_HOST = "8ebaa047-47d0-4765-a068-fb96a3b6d6b9";
constexpr auto CHAR_UUID_BACKEND_PORT = "4bb0157c-1c74-4cfb-81d8-351b895ce811";
constexpr auto CHAR_UUID_SAMPLE_DELAY = "a8222441-cb54-46d6-8765-173b3b1e06e2";

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server_ptr) {
    METEOS_LOG_LN("bt: client connected.");
  }
  void onDisconnect(BLEServer *server_ptr) {
    METEOS_LOG_LN("bt: client disconnected.");
  };
};

enum class CharacteristicType {
  wifi_ssid,
  wifi_pass,
  backend_host,
  backend_port,
  sample_delay
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  CharacteristicCallbacks(CharacteristicType type_, Config &cfg)
      : type{type_}, config{cfg} {}

 private:
  CharacteristicType type;
  Config &config;

  void onRead(BLECharacteristic *char_ptr) {
    switch (type) {
      case CharacteristicType::wifi_ssid:
        METEOS_LOG("bt: wifi ssid read: ");
        break;
      case CharacteristicType::wifi_pass:
        METEOS_LOG("bt: wifi pass read: ");
        break;
      case CharacteristicType::backend_host:
        METEOS_LOG("bt: backend host read: ");
        break;
      case CharacteristicType::backend_port:
        METEOS_LOG("bt: backend port read: ");
        break;
      case CharacteristicType::sample_delay:
        METEOS_LOG("bt: sample delay read: ");
        break;
    }
    METEOS_LOG_LN(char_ptr->getValue().c_str());
  };
  void onWrite(BLECharacteristic *char_ptr) {
    switch (type) {
      case CharacteristicType::wifi_ssid:
        config.wifi_ssid(char_ptr->getValue());
        METEOS_LOG_LN("bt: wifi ssid written");
        break;
      case CharacteristicType::wifi_pass:
        config.wifi_pass(char_ptr->getValue());
        METEOS_LOG_LN("bt: wifi pass written");
        break;
      case CharacteristicType::backend_host:
        config.backend_host(char_ptr->getValue());
        METEOS_LOG_LN("bt: backend host written");
        break;
      case CharacteristicType::backend_port:
        config.backend_port(String(char_ptr->getValue().c_str()).toInt());
        METEOS_LOG_LN("bt: backend port written");
        break;
      case CharacteristicType::sample_delay:
        config.sample_delay(
            std::chrono::seconds{String(char_ptr->getValue().c_str()).toInt()});
        METEOS_LOG_LN("bt: sample delay written");
        break;
    };
  };
};

void RadioBT::init() {
  METEOS_SCOPED_LOGGER("bt: init");

  // init GATT server, service and characteristics
  BLEDevice::init("Meteos Sensor");
  BLEServer *server_ptr = BLEDevice::createServer();

  server_ptr->setCallbacks(new ServerCallbacks());

  service_ptr = server_ptr->createService(SERVICE_UUID);
  auto rw =
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE;
  BLECharacteristic *char_wifi_ssid =
      service_ptr->createCharacteristic(CHAR_UUID_WIFI_SSID, rw);
  BLECharacteristic *char_wifi_pass =
      service_ptr->createCharacteristic(CHAR_UUID_WIFI_PASS, rw);
  BLECharacteristic *char_backend_host =
      service_ptr->createCharacteristic(CHAR_UUID_BACKEND_HOST, rw);
  BLECharacteristic *char_backend_port =
      service_ptr->createCharacteristic(CHAR_UUID_BACKEND_PORT, rw);
  BLECharacteristic *char_sample_delay =
      service_ptr->createCharacteristic(CHAR_UUID_SAMPLE_DELAY, rw);

  char_wifi_ssid->setValue(config.wifi_ssid());
  char_wifi_pass->setValue(config.wifi_pass());
  char_backend_host->setValue(config.backend_host());
  char_backend_port->setValue(String(config.backend_port()).c_str());
  char_sample_delay->setValue(
      String((int)config.sample_delay().count()).c_str());

  char_wifi_ssid->setCallbacks(
      new CharacteristicCallbacks(CharacteristicType::wifi_ssid, config));
  char_wifi_pass->setCallbacks(
      new CharacteristicCallbacks(CharacteristicType::wifi_pass, config));
  char_backend_host->setCallbacks(
      new CharacteristicCallbacks(CharacteristicType::backend_host, config));
  char_backend_port->setCallbacks(
      new CharacteristicCallbacks(CharacteristicType::backend_port, config));
  char_sample_delay->setCallbacks(
      new CharacteristicCallbacks(CharacteristicType::sample_delay, config));

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
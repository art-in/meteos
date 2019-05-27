#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "config.h"

#define SERVICE_UUID "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6"
#define SSID_CHARACTERISTIC_UUID "d1fe36b7-3223-4927-ada5-422f2b5afcf9"
#define PASS_CHARACTERISTIC_UUID "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9"

class RadioBT {
 public:
  RadioBT(Config &cfg) : config{cfg} {};
  void start_advertising();

 private:
  Config &config;
  BLEService *service_ptr;
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer);
  void onDisconnect(BLEServer *pServer);
};

class SSIDCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  SSIDCharacteristicCallbacks(Config &cfg) : config{cfg} {}

 private:
  Config &config;
  void onRead(BLECharacteristic *pCharacteristic);
  void onWrite(BLECharacteristic *pCharacteristic);
};

class PassCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  PassCharacteristicCallbacks(Config &cfg) : config{cfg} {}

 private:
  Config &config;
  void onRead(BLECharacteristic *pCharacteristic);
  void onWrite(BLECharacteristic *pCharacteristic);
};
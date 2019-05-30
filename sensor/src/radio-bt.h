#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "config.h"

class RadioBT {
 public:
  RadioBT(Config &cfg) : config{cfg} {};

  void init();
  void start_advertising();
  void stop_advertising();

 private:
  Config &config;
  BLEService *service_ptr;
  BLEAdvertising *advertising_ptr;
};

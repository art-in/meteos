#include "libs/Adafruit_BME280_Library/Adafruit_BME280.h"
#include "libs/MH-Z19-1.4.2/src/MHZ19.h"

#include "sample.h"

constexpr int UART_NUMBER = 2;

class Sensors {
 public:
  Sensors() : mhz_serial{UART_NUMBER} {};

  void init();
  Sample take_sample();
  Sample get_latest_sample();

 private:
  Adafruit_BME280 bme;
  MHZ19 mhz;
  HardwareSerial mhz_serial;
  Sample latest_sample;

  void init_bme();
  void init_mhz();
  float compensate_self_heating(float);
};
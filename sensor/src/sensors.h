#include "libs/Adafruit_BME280_Library/Adafruit_BME280.h"
#include "libs/MH-Z19-1.4.2/src/MHZ19.h"

#include "sample.h"

#define MHZ_BAUDRATE 9600
#define PASCAL_TO_MECURY_MM (0.00750062)

constexpr unsigned long CO2_WARMING_READ_PERIOD_MS = 30 * 1000L;

class Sensors {
 public:
  Sensors() : mhz_serial{2} {};

  void init(bool is_initial_boot);
  Sample take_sample();

  static Sample get_last_sample();

 private:
  Adafruit_BME280 bme;  // I2C
  MHZ19 mhz;            // UART
  HardwareSerial mhz_serial;

  void init_bme(bool is_initial_boot);
  void init_mhz(bool is_initial_boot);
};
#include <Arduino.h>
#include <chrono>

#include "sensors.h"
#include "utils.h"

constexpr int MHZ_BAUDRATE = 9600;
constexpr double PASCAL_TO_MECURY_MM = 0.00750062;
constexpr auto CO2_WARMING_READ_PERIOD = std::chrono::milliseconds{30000};

void Sensors::init() {
  init_bme();
  init_mhz();
}

void Sensors::init_bme() {
  METEOS_SCOPED_LOGGER("sensors: bme: init");

  if (!bme.begin(&Wire)) {
    METEOS_LOG_LN("sensors: bme: error: could not find BME280 sensor.");
    while (1)
      ;
  }

  // weather monitoring
  // suggested rate is 1/60Hz (1m)
  METEOS_LOG_LN("sensors: bme: set sampling config");
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,  // temperature
                  Adafruit_BME280::SAMPLING_X1,  // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_OFF);
}

void Sensors::init_mhz() {
  METEOS_SCOPED_LOGGER("sensors: mhz: init");

  mhz_serial.begin(MHZ_BAUDRATE);

  mhz.begin(mhz_serial);
  mhz.setFilter(true, true);

  mhz.setRange(2000);
  mhz.autoCalibration(true);

  {
    METEOS_SCOPED_LOGGER("sensors: mhz: warmup");
    while (true) {
      int co2 = mhz.getCO2(true, true);

      if (mhz.errorCode == RESULT_FILTER) {
        METEOS_LOG_LN("sensors: mhz: warming up...");
      } else if (mhz.errorCode != RESULT_OK) {
        METEOS_LOG_LN("sensors: mhz: failed to read CO2 on warmup.");
        return;
      } else if (co2 == 0) {
        // TODO: remove when zero co2 is filtered
        // https://github.com/WifWaf/MH-Z19/issues/6
        METEOS_LOG_LN("sensors: mhz: received zero, continuing warmup...");
      } else {
        // warmed up
        break;
      }

      delay(CO2_WARMING_READ_PERIOD.count());
    }
  }
}

Sample Sensors::take_sample() {
  METEOS_SCOPED_LOGGER("sensors: take sample");

  Sample s;

  bme.takeForcedMeasurement();

  // TODO: compensate in-case heat
  s.temperature = bme.readTemperature();
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MECURY_MM;

  s.co2 = mhz.getCO2(true, true);

  if (mhz.errorCode != RESULT_OK) {
    METEOS_LOG_LN("sensors: mhz: failed to read CO2. error: " +
                  String(mhz.errorCode));
  }

  latest_sample = s;
  return s;
}

Sample Sensors::get_latest_sample() { return latest_sample; }
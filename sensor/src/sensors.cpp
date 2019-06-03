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
  log_ln("sensors: bme: init...", true);
  auto before_ms = millis();

  if (!bme.begin(&Wire)) {
    log_ln("sensors: bme: error: could not find BME280 sensor.");
    while (1)
      ;
  }

  // weather monitoring
  // suggested rate is 1/60Hz (1m)
  log_ln("sensors: bme: set sampling config", true);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,  // temperature
                  Adafruit_BME280::SAMPLING_X1,  // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_OFF);

  log_ln("sensors: bme: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

void Sensors::init_mhz() {
  log_ln("sensors: mhz: init...", true);
  auto before_ms = millis();

  mhz_serial.begin(MHZ_BAUDRATE);

  // mhz.printCommunication(false, true);
  mhz.begin(mhz_serial);
  mhz.setFilter(true, true);

  mhz.setRange(2000);
  mhz.autoCalibration(true);

  log_ln("sensors: mhz: warming up...", true);
  while (true) {
    int co2 = mhz.getCO2(true, true);

    if (mhz.errorCode == RESULT_FILTER) {
      log_ln("sensors: mhz: warming up...", true);
    } else if (mhz.errorCode != RESULT_OK) {
      log_ln("sensors: mhz: failed to read CO2 on warmup.", true);
    } else if (co2 == 0) {
      // TODO: remove when zero co2 is filtered
      // https://github.com/WifWaf/MH-Z19/issues/6
      log_ln("sensors: mhz: received zero, continuing warmup...", true);
    } else {
      // warmed up
      break;
    }

    delay(CO2_WARMING_READ_PERIOD.count());
  }
  log_ln("sensors: mhz: warming up...done", true);

  log_ln("sensors: mhz: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

Sample Sensors::take_sample() {
  log_ln("sensors: taking sample...", true);
  auto before_ms = millis();

  Sample s;

  bme.takeForcedMeasurement();

  s.temperature = bme.readTemperature();
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MECURY_MM;

  s.co2 = mhz.getCO2(true, true);

  if (mhz.errorCode != RESULT_OK) {
    log_ln("sensors: mhz: failed to read CO2. error: " + String(mhz.errorCode),
           true);
  }

  latest_sample = s;

  log_ln(
      "sensors: taking sample...done in " + String(millis() - before_ms) + "ms",
      true);
  return s;
}

Sample Sensors::get_latest_sample() { return latest_sample; }
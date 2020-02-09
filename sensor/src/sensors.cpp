#include <Arduino.h>
#include <chrono>

#include "sensors.h"
#include "utils.h"

constexpr int MHZ_BAUDRATE = 9600;
constexpr double PASCAL_TO_MERCURY_MM = 0.00750062;
constexpr auto CO2_WARMING_PERIOD = std::chrono::seconds{30};

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

      delay(std::chrono::milliseconds{CO2_WARMING_PERIOD}.count());
    }
  }
}

Sample Sensors::take_sample() {
  METEOS_SCOPED_LOGGER("sensors: take sample");

  Sample s;

  bme.takeForcedMeasurement();

  s.temperature = compensate_self_heating(bme.readTemperature());
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MERCURY_MM;

  s.co2 = mhz.getCO2(true, true);

  if (mhz.errorCode != RESULT_OK) {
    METEOS_LOG_LN("sensors: mhz: failed to read CO2. error: " +
                  String(mhz.errorCode));
  }

  latest_sample = s;
  return s;
}

Sample Sensors::get_latest_sample() { return latest_sample; }

// compensates temperature reading mistake due to case self heating.
//
// esp32 radiates most of the heat, and some more comes from mh-z19 and battery.
// case is heating up gradually (mistake grows) for 1-2 hours after startup.
// mistake is different for usb and battery power supply (battery heats too).
//
// manual tests comparing readings from Sensor and external not encased bme280:
// - powering with esp32 dev board usb:
// -- ext.bme280 = 21.0 °C, Sensor = 23.0 °C, mistake = +2.0 °C
// -- ext.bme280 = 22.1 °C, Sensor = 24.0 °C, mistake = +1.9 °C
// -- ext.bme280 = 22.4 °C, Sensor = 24.4 °C, mistake = +2.0 °C
// -- ext.bme280 = 23.7 °C, Sensor = 25.8 °C, mistake = +2.1 °C
// -- ext.bme280 = 23.8 °C, Sensor = 26.0 °C, mistake = +2.2 °C
// -- ext.bme280 = 23.9 °C, Sensor = 25.8 °C, mistake = +1.9 °C
// -- ext.bme280 = 24.2 °C, Sensor = 26.2 °C, mistake = +2.0 °C
// -- ext.bme280 = 26.2 °C, Sensor = 27.8 °C, mistake = +1.6 °C
// -- ext.bme280 = 28.6 °C, Sensor = 30.4 °C, mistake = +1.8 °C
// - powering with battery:
// -- ext.bme280 = 26   °C, Sensor = 28.2 °C, mistake = +2.2 °C
//
// only add test data if readings are stabilized for at least 1 hour. this
// should help to avoid effect of inertia after startup for both sources, and
// also Sensor is generaly more inert to environment changes than ext.bme280.
// eg. readings from Sensor is the same (+-0.02 °C) for last hour and the same
// is for ext.bme280, or Sensor deviated +0.20 °C for last hour while ext.bme280
// also deviated +0.20 °C for last hour.
//
// compensating mistake for usb power supply instead of battery because usb is
// main usecase. for simplicity assuming that mistake is constant.
float Sensors::compensate_self_heating(float raw_temperature) {
  return raw_temperature - 2;
}
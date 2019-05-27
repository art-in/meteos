#include <Arduino.h>

#include "sensors.h"
#include "timings.h"
#include "utils.h"

constexpr unsigned long SAMPLE_DELAY_US = 10000000;  // us

RTC_DATA_ATTR Sample last_sample;

void Sensors::init(bool is_initial_boot) {
  init_bme(is_initial_boot);
  init_mhz(is_initial_boot);
}

void Sensors::init_bme(bool is_initial_boot) {
  log_ln("sensors: bme: init...", true);
  auto before_ms = millis();

  if (!bme.begin(&Wire)) {
    log_ln("sensors: bme: could not find BME280 sensor.");
    while (1)
      ;
  }

  if (is_initial_boot) {
    // weather monitoring
    // suggested rate is 1/60Hz (1m)
    log_ln("sensors: bme: set sampling config", true);
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,  // temperature
                    Adafruit_BME280::SAMPLING_X1,  // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_OFF);
  }

  log_ln("sensors: bme: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

void Sensors::init_mhz(bool is_initial_boot) {
  log_ln("sensors: mhz: init...", true);
  auto before_ms = millis();

  mhz_serial.begin(MHZ_BAUDRATE);

  // mhz.printCommunication(false, true);
  mhz.begin(mhz_serial);
  mhz.setFilter();

  if (is_initial_boot) {
    mhz.setRange(2000);
    mhz.autoCalibration(true);

    log_ln("sensors: mhz: warming up...", true);
    while (true) {
      int co2 = mhz.getCO2(false, true);

      if (mhz.errorCode == RESULT_FILTER) {
        log_ln("sensors: mhz: warming up...", true);
      } else if (mhz.errorCode != RESULT_OK) {
        log_ln("sensors: mhz: failed to read CO2 on warmup.", true);
      } else {
        log_ln("sensors: mhz: warmed up!", true);
        break;
      }

      delay(CO2_WARMING_READ_PERIOD_MS);
    }
    log_ln("sensors: mhz: warming up...done", true);
  }

  log_ln("sensors: mhz: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

Sample Sensors::take_sample() {
  log_ln("sensors: taking sample...", true);
  auto before_ms = millis();

  if (timings_next_sample_time_us) {
    int mistake_us = get_epoch_time_us() - timings_next_sample_time_us;
    log_ln("sensors: take sample time mistake: " +
           String(mistake_us / uS_TO_MS_FACTOR) + "ms");
  }

  timings_from_boot_to_sample_duration_us = millis() * uS_TO_MS_FACTOR;
  timings_next_sample_time_us = get_epoch_time_us() + SAMPLE_DELAY_US;

  Sample s;

  bme.takeForcedMeasurement();

  s.temperature = bme.readTemperature();
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MECURY_MM;

  s.co2 = mhz.getCO2(false, true);

  if (mhz.errorCode != RESULT_OK) {
    log_ln("sensors: mhz: failed to receive CO2. error: " + mhz.errorCode,
           true);
  }

  last_sample = s;

  log_ln(
      "sensors: taking sample...done in " + String(millis() - before_ms) + "ms",
      true);
  return s;
}

Sample Sensors::get_last_sample() { return last_sample; }
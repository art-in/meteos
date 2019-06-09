const express = require('express');
const moment = require('moment');
const linspace = require('array-linspace');

const PORT = 3000;
const HOST = '0.0.0.0';

const app = express();

const SAMPLE_PERIOD_MS = 5000;
const start = moment().subtract(1, 'week');

const TEMPERATURE_MIN = 26;
const TEMPERATURE_MAX = 29;
const TEMPERATURE_STEP = 0.01;

const HUMIDITY_MIN = 40;
const HUMIDITY_MAX = 50;
const HUMIDITY_STEP = 0.01;

const PRESSURE_MIN = 740;
const PRESSURE_MAX = 760;
const PRESSURE_STEP = 0.01;

const CO2_MIN = 450;
const CO2_MAX = 1000;
const CO2_STEP = 1;

app.get('/samples', (req, res) => {
  let from = req.query.from;
  const limit = req.query.limit || 1000;

  if (from) {
    from = moment(from);
  } else {
    from = moment(start);
  }

  let samples = [];
  const now = moment();

  const time = moment(start);
  let temperature = TEMPERATURE_MIN;
  let humidity = HUMIDITY_MIN;
  let pressure = PRESSURE_MIN;
  let co2 = CO2_MIN;

  let temperatureStep = TEMPERATURE_STEP;
  let humidityStep = HUMIDITY_STEP;
  let pressureStep = PRESSURE_STEP;
  let co2Step = CO2_STEP;

  while (now.diff(time, 'ms') >= 0) {
    temperature += temperatureStep;
    humidity += humidityStep;
    pressure += pressureStep;
    co2 += co2Step;

    if (temperature < TEMPERATURE_MIN || temperature > TEMPERATURE_MAX) {
      temperatureStep *= -1;
    }

    if (humidity < HUMIDITY_MIN || humidity > HUMIDITY_MAX) {
      humidityStep *= -1;
    }

    if (pressure < PRESSURE_MIN || pressure > PRESSURE_MAX) {
      pressureStep *= -1;
    }

    if (co2 < CO2_MIN || co2 > CO2_MAX) {
      co2Step *= -1;
    }

    if (time.diff(from, 'ms') >= 0) {
      samples.push({
        t: round(temperature, 2),
        h: round(humidity, 2),
        p: round(pressure, 2),
        c: round(co2, 2),
        u: time.format('YYYY-MM-DDTHH:mm:ss.SSS[Z]')
      });
    }

    time.add(SAMPLE_PERIOD_MS, 'ms');
  }

  if (samples.length > limit) {
    if (limit == 1) {
      samples = [samples[samples.length - 1]];
    } else {
      const indexes = new Set(
        linspace(0, samples.length - 1, limit).map(i => Math.floor(i))
      );
      samples = samples.filter((_, idx) => indexes.has(idx));
      console.log('samples.length', samples.length);
    }
  }

  res.send(samples);
});

function round(number, precision = 0) {
  const {pow, round} = Math;
  const divider = pow(10, precision);
  return round(number * divider) / divider;
}

app.listen(PORT, HOST, () => {
  console.log(`Backend mock is listening on http://${HOST}:${PORT}`);
});

const express = require('express');
const moment = require('moment');

const PORT = 3000;
const HOST = '0.0.0.0';

const app = express();

const SAMPLE_PERIOD_MS = 5000;
const start = moment();

app.get('/samples', (req, res) => {
  let from = req.query.from;

  if (from) {
    from = moment(from);
  } else {
    from = moment(start);
  }

  const samples = [];
  const now = moment();

  const time = moment(start);
  let temperature = 26;
  let humidity = 40;
  let pressure = 740;
  let co2 = 750;

  while (now.diff(time, 'ms') >= 0) {
    temperature += 0.1;
    humidity += 0.3;
    pressure += 0.3;
    co2 += 0.3;

    if (time.diff(from, 'ms') >= 0) {
      samples.push({
        t: Math.floor(temperature),
        h: Math.floor(humidity),
        p: Math.floor(pressure),
        c: Math.floor(co2),
        u: time.format()
      });
    }

    time.add(SAMPLE_PERIOD_MS, 'ms');
  }

  res.send(samples);
});

app.listen(PORT, HOST, () => {
  console.log(`Backend mock is listening on http://${HOST}:${PORT}`);
});

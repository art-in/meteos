export default class Sample {
  time;
  temperature;
  humidity;
  pressure;
  co2;

  constructor(
    time = null,
    temperature = null,
    humidity = null,
    pressure = null,
    co2 = null
  ) {
    this.time = time;
    this.temperature = temperature;
    this.humidity = humidity;
    this.pressure = pressure;
    this.co2 = co2;
  }
}

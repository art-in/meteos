import PeriodType from './PeriodType';

const STORAGE_PREFIX = '[meteos] ';

const storageKeys = {
  chartPeriod: STORAGE_PREFIX + 'chart_period',
  isTemperatureActive: STORAGE_PREFIX + 'temp_active',
  isHumidityActive: STORAGE_PREFIX + 'hum_active',
  isPressureActive: STORAGE_PREFIX + 'pres_active',
  isCO2Active: STORAGE_PREFIX + 'co2_active'
};

const defaults = {
  chartPeriod: PeriodType.day,
  isTemperatureActive: true,
  isHumidityActive: true,
  isPressureActive: true,
  isCO2Active: true
};

function get(key, defaultValue) {
  const value = localStorage[key];
  return value == null ? defaultValue : JSON.parse(value);
}

function set(key, value) {
  try {
    localStorage[key] = JSON.stringify(value);
  } catch (e) {
    // quota exceed, user disabled local storage, etc.
  }
}

function createGetterSetter(name) {
  let value = get(storageKeys[name], defaults[name]);

  return newValue => {
    if (newValue !== undefined) {
      value = newValue;
      set(storageKeys[name], newValue);
    }

    return value;
  };
}

export const chartPeriod = createGetterSetter('chartPeriod');
export const isTemperatureActive = createGetterSetter('isTemperatureActive');
export const isHumidityActive = createGetterSetter('isHumidityActive');
export const isPressureActive = createGetterSetter('isPressureActive');
export const isCO2Active = createGetterSetter('isCO2Active');

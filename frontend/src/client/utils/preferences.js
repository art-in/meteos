import PeriodType from './PeriodType';

const STORAGE_PREFIX = 'meteos_';
const STORAGE_KEY_CHART_PERIOD = STORAGE_PREFIX + 'chart_period';

const prefs = {
  chartPeriod: get(STORAGE_KEY_CHART_PERIOD, PeriodType.day)
};

function get(key, defaultValue) {
  const value = localStorage[key];
  return value == null ? defaultValue : value;
}

function set(key, value) {
  try {
    localStorage[key] = value;
  } catch (e) {
    // quota exceed, user disabled local storage, etc.
  }
}

export function chartPeriod(value) {
  if (value !== undefined) {
    prefs.chartPeriod = value;
    set(STORAGE_KEY_CHART_PERIOD, value);
  }

  return prefs.chartPeriod;
}

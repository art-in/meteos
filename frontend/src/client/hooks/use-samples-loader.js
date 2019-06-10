import {useState, useEffect} from 'react';
import moment from 'moment';

import usePrevious from 'hooks/use-previous';
import LoadStatusType from 'utils/LoadStatusType';
import * as api from 'utils/api';

const RELOAD_DELAY = 10 * 1000; // ms
const ACTUAL_SAMPLE_THRESHOLD = 60 * 1000; // ms
const SAMPLES_LIMIT = 500;

/**
 * Loads samples periodically for specified period.
 *
 * @param {string} period - last hour/week/month/etc.
 */
export default function useSamplesLoader(period) {
  const [loadStatus, setLoadStatus] = useState(LoadStatusType.connected);
  const [loadedSamples, setLoadedSamples] = useState([]);
  const [actualSample, setActualSample] = useState();
  const prevPeriod = usePrevious(period);

  useEffect(() => {
    async function load(isCleanReload) {
      const now = moment();
      const periodStart = moment(now).subtract(moment.duration(1, period));
      const periodDurationMs = now.diff(periodStart);
      const prevLatestSample = loadedSamples[loadedSamples.length - 1];

      // only load full period once, then start from latest loaded sample
      let from;
      if (
        prevLatestSample &&
        periodStart.isBefore(prevLatestSample.time) &&
        !isCleanReload
      ) {
        from = moment(prevLatestSample.time).add(1, 'ms');
      } else {
        from = periodStart;
      }

      let samples;
      try {
        samples = await api.getSamples({from, limit: SAMPLES_LIMIT});
      } catch (e) {
        setLoadStatus(LoadStatusType.disconnected);
        setActualSample();

        // hack: trigger effect to restart timer
        setLoadedSamples([...loadedSamples]);
        return;
      }

      // get latest sample before time gap filtering, for setting actual sample
      let latestSample = prevLatestSample;
      if (samples.length) {
        latestSample = samples[samples.length - 1];
      }

      // keep time gap between samples, so count limit is not exceeded.
      // we need to do it even though we already limiting api response, since
      // target period may have "holes" with no samples, so we may receive
      // over-loaded sub-periods.
      const gapMs = periodDurationMs / SAMPLES_LIMIT;
      let prevSample = isCleanReload ? null : prevLatestSample;
      samples = samples.filter(sample => {
        if (!prevSample || sample.timeMs - prevSample.timeMs >= gapMs) {
          prevSample = sample;
          return true;
        }
      });

      if (!isCleanReload) {
        samples = loadedSamples.concat(samples);
      }

      // remove unused samples outside period to prevent memory leak
      const firstSampleInsidePeriodIdx = samples.findIndex(sample =>
        periodStart.isSameOrBefore(sample.time)
      );
      samples = samples.slice(firstSampleInsidePeriodIdx);

      setLoadedSamples(samples);

      if (samples.length > SAMPLES_LIMIT) {
        console.warn(
          `Samples count exceeded limit (${SAMPLES_LIMIT}): ${samples.length}`
        );
      }

      if (
        !latestSample ||
        now.diff(latestSample.time) > ACTUAL_SAMPLE_THRESHOLD
      ) {
        setLoadStatus(LoadStatusType.outdated);
        setActualSample();
      } else {
        setLoadStatus(LoadStatusType.connected);
        setActualSample(latestSample);
      }
    }

    let timerId;

    if (period != prevPeriod) {
      load(true);
    } else {
      timerId = setTimeout(load, RELOAD_DELAY);
    }

    return () => clearTimeout(timerId);
  }, [period, prevPeriod, loadedSamples]);

  return {loadStatus, samples: loadedSamples, actualSample};
}

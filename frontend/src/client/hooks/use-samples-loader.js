import {useState, useEffect} from 'react';
import moment from 'moment';

import usePrevious from 'hooks/use-previous';
import LoadStatusType from 'utils/LoadStatusType';
import * as api from 'utils/api';

const RELOAD_DELAY = 5000; // ms
const OUTDATED_TRASHOLD = 10000; // ms

/**
 * Loads samples periodically for specified period.
 *
 * @param {string} period - last hour/week/month/etc.
 */
export default function useSamplesLoader(period) {
  const [loadStatus, setLoadStatus] = useState(LoadStatusType.disconnected);
  const [loadedSamples, setLoadedSamples] = useState([]);
  const [actualSample, setActualSample] = useState();
  const prevPeriod = usePrevious(period);

  useEffect(() => {
    async function load(isCleanReload) {
      const now = moment();
      const periodStart = moment(now).subtract(moment.duration(1, period));

      // only load full period once, then start from latest loaded sample
      let from;
      if (loadedSamples.length && !isCleanReload) {
        const latestSample = loadedSamples[loadedSamples.length - 1];
        from = moment(latestSample.time).add(1, 'ms');
      } else {
        from = periodStart;
      }

      let samples;
      try {
        samples = await api.getSamples({from});
      } catch (e) {
        setLoadStatus(LoadStatusType.disconnected);
        setActualSample();

        // hack: trigger effect to restart timer
        setLoadedSamples([...loadedSamples]);
        return;
      }

      if (!isCleanReload) {
        samples = loadedSamples.concat(samples);
      }

      // remove unused samples outside period to prevent memory leak
      const firstSampleInsidePeriodIdx = samples.findIndex(sample =>
        periodStart.isSameOrBefore(sample.time)
      );
      samples = samples.slice(firstSampleInsidePeriodIdx);

      setLoadedSamples(samples);

      if (samples) {
        const latestSample = samples[samples.length - 1];

        if (
          !latestSample ||
          now.diff(latestSample.time, 'ms') > OUTDATED_TRASHOLD
        ) {
          setLoadStatus(LoadStatusType.outdated);
          setActualSample();
        } else {
          setLoadStatus(LoadStatusType.connected);
          setActualSample(latestSample);
        }
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

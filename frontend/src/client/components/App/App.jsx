import React, {useState, useCallback} from 'react';
import cn from 'classnames';

import useSamplesLimit from 'hooks/use-samples-limit';
import useSamplesLoader from 'hooks/use-samples-loader';
import * as preferences from 'utils/preferences';
import ReadingType from 'utils/ReadingType';
import Header from '../Header';
import Readings from '../Readings';
import Chart from '../Chart';
import Footer from '../Footer';
import Modal from '../Modal';
import ConfigForm from '../ConfigForm';
import classes from './App.css';

require('./shared/global.css');
require('./shared/fonts.css');

export default function App() {
  const [chartPeriod, setChartPeriod] = useState(preferences.chartPeriod());
  const [isTemperatureActive, setIsTemperatureActive] = useState(
    preferences.isTemperatureActive()
  );
  const [isHumidityActive, setIsHumidityActive] = useState(
    preferences.isHumidityActive()
  );
  const [isPressureActive, setIsPressureActive] = useState(
    preferences.isPressureActive()
  );
  const [isCO2Active, setIsCO2Active] = useState(preferences.isCO2Active());
  const [isConfigOpened, setIsConfigOpened] = useState(false);
  const {samplesLimit} = useSamplesLimit();
  const {loadStatus, samples, actualSample} = useSamplesLoader(
    chartPeriod,
    samplesLimit
  );

  const onChartPeriodChange = useCallback(period => {
    preferences.chartPeriod(period);
    setChartPeriod(period);
  }, []);

  const onReadingActivationChange = useCallback((reading, isActive) => {
    switch (reading) {
      case ReadingType.temperature:
        preferences.isTemperatureActive(isActive);
        setIsTemperatureActive(isActive);
        break;
      case ReadingType.humidity:
        preferences.isHumidityActive(isActive);
        setIsHumidityActive(isActive);
        break;
      case ReadingType.pressure:
        preferences.isPressureActive(isActive);
        setIsPressureActive(isActive);
        break;
      case ReadingType.co2:
        preferences.isCO2Active(isActive);
        setIsCO2Active(isActive);
        break;
      default:
        throw Error(`Unknown reading type '${reading}'`);
    }
  }, []);

  const onConfigOpen = useCallback(() => {
    setIsConfigOpened(true);
  }, []);

  const onConfigClose = useCallback(() => {
    setIsConfigOpened(false);
  }, []);

  return (
    <div
      className={cn(classes.root, {
        [classes['modal-opened']]: isConfigOpened
      })}
    >
      <Header
        className={classes.header}
        status={loadStatus}
        onConfigOpen={onConfigOpen}
      />
      <Readings
        className={classes.readings}
        sample={actualSample}
        isTemperatureActive={isTemperatureActive}
        isHumidityActive={isHumidityActive}
        isPressureActive={isPressureActive}
        isCO2Active={isCO2Active}
        onReadingActivationChange={onReadingActivationChange}
      />
      <Chart
        className={classes.chart}
        samples={samples}
        period={chartPeriod}
        isTemperatureActive={isTemperatureActive}
        isHumidityActive={isHumidityActive}
        isPressureActive={isPressureActive}
        isCO2Active={isCO2Active}
        onPeriodChange={onChartPeriodChange}
      />
      <Footer className={classes.footer} />

      {isConfigOpened && (
        <Modal onClose={onConfigClose}>
          <ConfigForm />
        </Modal>
      )}
    </div>
  );
}

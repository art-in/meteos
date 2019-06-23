import React, {useState, useCallback} from 'react';
import cn from 'classnames';

import useSamplesLoader from 'hooks/use-samples-loader';
import * as preferences from 'utils/preferences';
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
  const [isConfigOpened, setIsConfigOpened] = useState(false);
  const {loadStatus, samples, actualSample} = useSamplesLoader(chartPeriod);

  const onChartPeriodChange = useCallback(period => {
    preferences.chartPeriod(period);
    setChartPeriod(period);
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
      <Readings className={classes.readings} sample={actualSample} />
      <Chart
        className={classes.chart}
        samples={samples}
        period={chartPeriod}
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

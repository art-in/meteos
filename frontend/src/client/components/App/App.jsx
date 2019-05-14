import React, {useState, useCallback} from 'react';

import useSamplesLoader from 'hooks/use-samples-loader';
import PeriodType from 'utils/PeriodType';
import Header from '../Header';
import Readings from '../Readings';
import Chart from '../Chart';
import Footer from '../Footer';
import classes from './App.css';

require('./shared/global.css');
require('./shared/fonts.css');

export default function App() {
  const [chartPeriod, setChartPeriod] = useState(PeriodType.hour);
  const {loadStatus, samples, actualSample} = useSamplesLoader(chartPeriod);

  const onChartPeriodChange = useCallback(period => {
    setChartPeriod(period);
  }, []);

  return (
    <div className={classes.root}>
      <Header className={classes.header} status={loadStatus} />
      <Readings className={classes.readings} sample={actualSample} />
      <Chart
        className={classes.chart}
        samples={samples}
        period={chartPeriod}
        onPeriodChange={onChartPeriodChange}
      />
      <Footer className={classes.footer} />
    </div>
  );
}

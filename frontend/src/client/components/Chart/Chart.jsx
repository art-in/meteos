import React, {useCallback} from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';
import moment from 'moment';
import {
  ResponsiveContainer,
  XAxis,
  YAxis,
  LineChart,
  Line,
  CartesianGrid,
  Tooltip
} from 'recharts';

import Sample from 'utils/Sample';
import PeriodType from 'utils/PeriodType';
import PeriodSelector from '../PeriodSelector';
import classes from './Chart.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  samples: PropTypes.arrayOf(PropTypes.instanceOf(Sample)).isRequired,
  period: PropTypes.oneOf(Object.values(PeriodType)).isRequired,

  onPeriodChange: PropTypes.func.isRequired
};

export default function Chart({className, samples, period, onPeriodChange}) {
  const periodStartMs = Number(moment().subtract(1, period));
  const periodEndMs = Number(moment());

  // TODO: remove when recharts bug is fixed
  // https://github.com/recharts/recharts/issues/1493
  if (!samples.length) {
    samples = [new Sample(1, 1, 1, 1, 1)];
  }

  const formatLabel = useCallback(
    time => moment(time).format('dddd, MMMM D, HH:mm:ss'),
    []
  );

  const formatXAxisTick = useCallback(
    time => {
      switch (period) {
        case 'minute':
          return moment(time).format('mm:ss');
        case 'hour':
          return moment(time).format('HH:mm:ss');
        case 'day':
          return moment(time).format('HH:mm:ss');
        case 'week':
          return moment(time).format('MMMM D, HH:mm:ss');
        default:
          throw Error(`Unknown period '${period}'`);
      }
    },
    [period]
  );

  return (
    <div className={cn(className, classes.root)}>
      <PeriodSelector
        className={classes.periodSelector}
        period={period}
        onPeriodChange={onPeriodChange}
      />

      <div className={classes.count}>samples count: {samples.length}</div>

      <ResponsiveContainer>
        <LineChart data={samples}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis
            dataKey="timeMs"
            interval={'preserveStartEnd'}
            tickFormatter={formatXAxisTick}
            type="number"
            domain={[periodStartMs, periodEndMs]}
          />

          <YAxis yAxisId="temperature" type="number" domain={[25, 30]} hide />
          <YAxis yAxisId="humidity" type="number" domain={[30, 60]} hide />
          <YAxis yAxisId="pressure" type="number" domain={[730, 770]} hide />
          <YAxis yAxisId="co2" type="number" domain={[400, 1300]} hide />

          <Tooltip
            isAnimationActive={false}
            labelFormatter={formatLabel}
            labelStyle={{paddingBottom: 10, color: 'gray'}}
            itemStyle={{paddingRight: 20}}
          />

          <Line
            dataKey="co2"
            yAxisId="co2"
            type="basis"
            isAnimationActive={false}
            dot={{fill: '#006400', r: 1.5}}
            activeDot={{stroke: 'red', r: 1.5}}
            stroke="none"
            unit=" ppm"
          />
          <Line
            dataKey="pressure"
            yAxisId="pressure"
            type="basis"
            isAnimationActive={false}
            dot={{fill: '#222222', r: 1.5}}
            activeDot={{stroke: 'red', r: 1.5}}
            stroke="none"
            unit=" mm hg"
          />
          <Line
            dataKey="humidity"
            yAxisId="humidity"
            type="basis"
            isAnimationActive={false}
            dot={{fill: '#0000ff', r: 1.5}}
            activeDot={{stroke: 'red', r: 1.5}}
            stroke="none"
            unit=" %"
          />
          <Line
            dataKey="temperature"
            yAxisId="temperature"
            type="basis"
            isAnimationActive={false}
            dot={{fill: '#ff0000', r: 1.5}}
            activeDot={{stroke: 'red', r: 1.5}}
            stroke="none"
            unit=" °C"
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

Chart.propTypes = propTypes;

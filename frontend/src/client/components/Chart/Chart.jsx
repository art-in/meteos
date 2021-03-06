import React, {useCallback} from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';
import moment from 'moment';
import {
  ResponsiveContainer,
  XAxis,
  YAxis,
  AreaChart,
  Area,
  CartesianGrid,
  Tooltip
} from 'recharts';

import colors from 'utils/colors';
import units from 'utils/units';
import Sample from 'utils/Sample';
import PeriodType from 'utils/PeriodType';
import PeriodSelector from '../PeriodSelector';
import classes from './Chart.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  samples: PropTypes.arrayOf(PropTypes.instanceOf(Sample)).isRequired,
  period: PropTypes.oneOf(Object.values(PeriodType)).isRequired,
  isTemperatureActive: PropTypes.bool.isRequired,
  isHumidityActive: PropTypes.bool.isRequired,
  isPressureActive: PropTypes.bool.isRequired,
  isCO2Active: PropTypes.bool.isRequired,

  onPeriodChange: PropTypes.func.isRequired
};

function Chart({
  className,
  samples,
  period,
  isTemperatureActive,
  isHumidityActive,
  isPressureActive,
  isCO2Active,
  onPeriodChange
}) {
  const now = moment();

  const periodStartMs = Number(moment(now).subtract(1, period));
  const periodEndMs = Number(now);

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
          return moment(time).format('dddd, MMMM D');
        case 'month':
          return moment(time).format('MMMM D');
        case 'year':
          return moment(time).format('MMMM');
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

      <ResponsiveContainer>
        <AreaChart data={samples}>
          <defs>
            <linearGradient id="gradientCO2" x1="0" y1="0" x2="0" y2="1">
              <stop
                offset="0%"
                stopColor={colors.co2.secondary}
                stopOpacity={0.4}
              />
              <stop
                offset="30%"
                stopColor={colors.co2.secondary}
                stopOpacity={0}
              />
            </linearGradient>
            <linearGradient id="gradientPressure" x1="0" y1="0" x2="0" y2="1">
              <stop
                offset="0%"
                stopColor={colors.pressure.secondary}
                stopOpacity={0.2}
              />
              <stop
                offset="30%"
                stopColor={colors.pressure.secondary}
                stopOpacity={0}
              />
            </linearGradient>
            <linearGradient id="gradientHumidity" x1="0" y1="0" x2="0" y2="1">
              <stop
                offset="0%"
                stopColor={colors.humidity.secondary}
                stopOpacity={0.2}
              />
              <stop
                offset="30%"
                stopColor={colors.humidity.secondary}
                stopOpacity={0}
              />
            </linearGradient>
            <linearGradient
              id="gradientTemperature"
              x1="0"
              y1="0"
              x2="0"
              y2="1"
            >
              <stop
                offset="0%"
                stopColor={colors.temperature.secondary}
                stopOpacity={0.4}
              />
              <stop
                offset="30%"
                stopColor={colors.temperature.secondary}
                stopOpacity={0}
              />
            </linearGradient>
          </defs>

          <CartesianGrid strokeDasharray="3 3" stroke="#808080" />
          <XAxis
            dataKey="timeMs"
            interval={'preserveStartEnd'}
            tickFormatter={formatXAxisTick}
            type="number"
            domain={[periodStartMs, periodEndMs]}
          />

          <YAxis yAxisId="temperature" type="number" domain={[18, 28]} hide />
          <YAxis yAxisId="humidity" type="number" domain={[15, 60]} hide />
          <YAxis yAxisId="pressure" type="number" domain={[740, 780]} hide />
          <YAxis yAxisId="co2" type="number" domain={[400, 1500]} hide />

          {/* hide tooltip if no samples to avoid blinking line on the left side */}
          {samples.length && (
            <Tooltip
              isAnimationActive={false}
              labelFormatter={formatLabel}
              labelStyle={{
                paddingBottom: 10,
                color: '#808080'
              }}
              contentStyle={{
                backgroundColor: '#1f1d1d',
                border: '2px solid #808080'
              }}
              itemStyle={{paddingRight: 20}}
            />
          )}

          {isTemperatureActive && (
            <Area
              dataKey="temperature"
              yAxisId="temperature"
              type="basis"
              isAnimationActive={false}
              fill="url(#gradientTemperature)"
              fillOpacity={1}
              dot={{stroke: colors.temperature.primary, r: 0.8}}
              activeDot={{stroke: '#fff', r: 1.5}}
              stroke={colors.temperature.secondary}
              unit={' ' + units.temperature}
            />
          )}
          {isHumidityActive && (
            <Area
              dataKey="humidity"
              yAxisId="humidity"
              type="basis"
              isAnimationActive={false}
              fill="url(#gradientHumidity)"
              fillOpacity={1}
              dot={{stroke: colors.humidity.primary, r: 0.8}}
              activeDot={{stroke: '#fff', r: 1.5}}
              stroke={colors.humidity.secondary}
              unit={' ' + units.humidity}
            />
          )}
          {isPressureActive && (
            <Area
              dataKey="pressure"
              yAxisId="pressure"
              type="basis"
              isAnimationActive={false}
              fill="url(#gradientPressure)"
              fillOpacity={1}
              dot={{stroke: colors.pressure.primary, r: 0.8}}
              activeDot={{stroke: '#fff', r: 1.5}}
              stroke={colors.pressure.secondary}
              unit={' ' + units.pressure}
            />
          )}
          {isCO2Active && (
            <Area
              dataKey="co2"
              yAxisId="co2"
              type="basis"
              isAnimationActive={false}
              fill="url(#gradientCO2)"
              fillOpacity={1}
              dot={{stroke: colors.co2.primary, r: 0.8}}
              activeDot={{stroke: '#fff', r: 1.5}}
              stroke={colors.co2.secondary}
              unit={' ' + units.co2}
            />
          )}
        </AreaChart>
      </ResponsiveContainer>
    </div>
  );
}

Chart.propTypes = propTypes;

export default React.memo(Chart);

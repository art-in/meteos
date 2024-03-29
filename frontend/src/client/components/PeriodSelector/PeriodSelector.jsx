import React from 'react';
import PropTypes from 'prop-types';
import cn from 'classnames';

import PeriodType from 'utils/PeriodType';
import classes from './PeriodSelector.css';

const propTypes = {
  className: PropTypes.string,
  period: PropTypes.oneOf(Object.values(PeriodType)).isRequired,
  onPeriodChange: PropTypes.func.isRequired
};

function PeriodSelector({className, period, onPeriodChange}) {
  return (
    <select
      className={cn(className, classes.root)}
      value={period}
      onChange={event => onPeriodChange(event.target.value)}
    >
      <option value={PeriodType.minute}>last minute</option>
      <option value={PeriodType.hour}>last hour</option>
      <option value={PeriodType.day}>last day</option>
      <option value={PeriodType.week}>last week</option>
      <option value={PeriodType.month}>last month</option>
      <option value={PeriodType.year}>last year</option>
    </select>
  );
}

PeriodSelector.propTypes = propTypes;

export default React.memo(PeriodSelector);

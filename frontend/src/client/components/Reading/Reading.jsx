import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import colors from 'utils/colors.js';
import units from 'utils/units.js';
import classes from './Reading.css';

const titles = {
  temperature: 'Temperature',
  humidity: 'Humidity',
  pressure: 'Pressure',
  co2: 'CO2'
};

const propTypes = {
  className: PropTypes.string.isRequired,
  type: PropTypes.oneOf(['temperature', 'humidity', 'pressure', 'co2'])
    .isRequired,
  value: PropTypes.number
};

function Reading({className, type, value}) {
  const title = titles[type];
  const unit = units[type];
  const color = colors[type];

  return (
    <div
      className={cn(className, classes.root, classes[type])}
      style={{borderColor: color.primary, backgroundColor: color.secondary}}
    >
      <div className={classes.title}>{title}</div>
      <div className={classes.valueContainer}>
        <span className={classes.value}>{value == null ? '?' : value}</span>
        <span className={classes.unit}>{unit}</span>
      </div>
    </div>
  );
}

Reading.propTypes = propTypes;

export default React.memo(Reading);

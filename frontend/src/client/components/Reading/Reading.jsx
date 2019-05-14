import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import classes from './Reading.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  title: PropTypes.string.isRequired,
  value: PropTypes.number,
  valueSuffix: PropTypes.string
};

export default function Reading({className, title, value, valueSuffix}) {
  return (
    <div className={cn(className, classes.root)}>
      <div className={classes.title}>{title}</div>
      <div className={classes.valueContainer}>
        <span>{value == null ? '?' : value}</span>
        <span className={classes.valueSuffix}>{valueSuffix}</span>
      </div>
    </div>
  );
}

Reading.propTypes = propTypes;

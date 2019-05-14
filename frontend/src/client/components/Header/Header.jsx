import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import LoadStatus from '../LoadStatus';
import classes from './Header.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  status: PropTypes.number.isRequired
};

export default function Header({className, status}) {
  return (
    <div className={cn(className, classes.root)}>
      <LoadStatus status={status} />
    </div>
  );
}

Header.propTypes = propTypes;

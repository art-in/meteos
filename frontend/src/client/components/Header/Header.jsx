import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import LoadStatus from '../LoadStatus';
import ConfigIcon from './icons/config.svg';
import classes from './Header.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  status: PropTypes.number.isRequired,
  onConfigOpen: PropTypes.func.isRequired
};

function Header({className, status, onConfigOpen}) {
  return (
    <div className={cn(className, classes.root)}>
      <LoadStatus status={status} />

      <ConfigIcon
        height={20}
        width={20}
        className={classes.config}
        onClick={onConfigOpen}
      />
    </div>
  );
}

Header.propTypes = propTypes;

export default React.memo(Header);

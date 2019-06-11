import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import classes from './Footer.css';

const propTypes = {
  className: PropTypes.string.isRequired
};

function Footer({className}) {
  return <div className={cn(className, classes.root)}>meteos</div>;
}

Footer.propTypes = propTypes;

export default React.memo(Footer);

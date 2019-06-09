import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';
import moment from 'moment';

import Reading from '../Reading';
import Sample from 'utils/Sample';
import classes from './Readings.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  sample: PropTypes.instanceOf(Sample)
};

const defaultProps = {
  sample: new Sample()
};

export default function Readings({className, sample}) {
  const title = sample.time
    ? `Last update: ${moment(sample.time).format('HH:mm:ss')}`
    : '';

  return (
    <div className={cn(className, classes.root)} title={title}>
      <Reading
        className={classes.reading}
        type="temperature"
      
        value={sample.temperature}
      
      />
      <Reading
        className={classes.reading}
        type="humidity"
      
        value={sample.humidity}
      
      />
      <Reading
        className={classes.reading}
        type="pressure"
        
        value={sample.pressure}
 
      />
      <Reading
        className={classes.reading}
        type="co2"
      
        value={sample.co2}
      
      />
    </div>
  );
}

Readings.propTypes = propTypes;
Readings.defaultProps = defaultProps;

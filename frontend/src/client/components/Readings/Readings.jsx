import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

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
  return (
    <div className={cn(className, classes.root)}>
      <Reading
        className={classes.reading}
        title="Temperature"
        value={sample.temperature}
        valueSuffix={'°C'}
      />
      <Reading
        className={classes.reading}
        title="Humidity"
        value={sample.humidity}
        valueSuffix={'%'}
      />
      <Reading
        className={classes.reading}
        title="Pressure"
        value={sample.pressure}
        valueSuffix={'mm hg'}
      />
      <Reading
        className={classes.reading}
        title="CO2"
        value={sample.co2}
        valueSuffix={'ppm'}
      />
    </div>
  );
}

Readings.propTypes = propTypes;
Readings.defaultProps = defaultProps;

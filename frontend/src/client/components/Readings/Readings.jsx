import React from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';
import moment from 'moment';

import Sample from 'utils/Sample';
import ReadingType from 'utils/ReadingType';
import Reading from '../Reading';
import classes from './Readings.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  sample: PropTypes.instanceOf(Sample),
  isTemperatureActive: PropTypes.bool.isRequired,
  isHumidityActive: PropTypes.bool.isRequired,
  isPressureActive: PropTypes.bool.isRequired,
  isCO2Active: PropTypes.bool.isRequired,

  onReadingActivationChange: PropTypes.func.isRequired
};

const defaultProps = {
  sample: new Sample()
};

function Readings({
  className,
  sample,
  isTemperatureActive,
  isHumidityActive,
  isPressureActive,
  isCO2Active,
  onReadingActivationChange
}) {
  const title = sample.time
    ? `Last update: ${moment(sample.time).format('HH:mm:ss')}`
    : '';

  return (
    <div className={cn(className, classes.root)} title={title}>
      <Reading
        className={classes.reading}
        type={ReadingType.temperature}
        isActive={isTemperatureActive}
        value={sample.temperature}
        onReadingActivationChange={onReadingActivationChange}
      />
      <Reading
        className={classes.reading}
        type={ReadingType.humidity}
        isActive={isHumidityActive}
        value={sample.humidity}
        onReadingActivationChange={onReadingActivationChange}
      />
      <Reading
        className={classes.reading}
        type={ReadingType.pressure}
        isActive={isPressureActive}
        value={sample.pressure}
        onReadingActivationChange={onReadingActivationChange}
      />
      <Reading
        className={classes.reading}
        type={ReadingType.co2}
        isActive={isCO2Active}
        value={sample.co2}
        onReadingActivationChange={onReadingActivationChange}
      />
    </div>
  );
}

Readings.propTypes = propTypes;
Readings.defaultProps = defaultProps;

export default React.memo(Readings);

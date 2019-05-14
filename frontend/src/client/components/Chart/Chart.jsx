import React, {useRef, useEffect} from 'react';
import cn from 'classnames';
import PropTypes from 'prop-types';

import Sample from 'utils/Sample';
import PeriodType from 'utils/PeriodType';
import classes from './Chart.css';

const propTypes = {
  className: PropTypes.string.isRequired,
  samples: PropTypes.arrayOf(PropTypes.instanceOf(Sample)).isRequired,
  period: PropTypes.oneOf(Object.values(PeriodType)).isRequired,

  onPeriodChange: PropTypes.func.isRequired
};

export default function Chart({className, samples, period, onPeriodChange}) {
  const listRef = useRef(null);

  useEffect(() => {
    const div = listRef.current;
    div.scrollTop = div.scrollHeight;
  }, [samples.length]);

  return (
    <div className={cn(className, classes.root)}>
      <select
        className={classes.periodSelect}
        value={period}
        onChange={event => onPeriodChange(event.target.value)}
      >
        <option value="hour">hour</option>
        <option value="day">day</option>
        <option value="week">week</option>
      </select>

      <div className={classes.list} ref={listRef}>
        {samples.map(s => (
          <div key={s.time}>
            {s.time} - {s.temperature} {s.humidity} {s.pressure} {s.co2}
          </div>
        ))}
      </div>

      <div style={{padding: 50}}>samples count: {samples.length}</div>
    </div>
  );
}

Chart.propTypes = propTypes;

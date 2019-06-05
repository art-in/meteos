import React from 'react';
import PropTypes from 'prop-types';

import CloseIcon from './icons/close.svg';
import classes from './Modal.css';

const propTypes = {
  children: PropTypes.node,
  onClose: PropTypes.func
};

export default function Modal({children, onClose}) {
  return (
    <div className={classes.root} onClick={onClose}>
      <div className={classes.content} onClick={e => e.stopPropagation()}>
        {children}
        <CloseIcon className={classes.close} width={20} onClick={onClose} />
      </div>
    </div>
  );
}

Modal.propTypes = propTypes;

import React from 'react';
import PropTypes from 'prop-types';

import LoadStatusType from 'utils/LoadStatusType';
import ConnectedIcon from './icons/connected.svg';
import OutdatedIcon from './icons/outdated.svg';
import DisconnectedIcon from './icons/disconnected.svg';

const propTypes = {
  status: PropTypes.oneOf(Object.values(LoadStatusType)).isRequired
};

export default function LoadStatus({status}) {
  let title;
  let icon;

  switch (status) {
    case LoadStatusType.connected:
      title = 'Connected. All good.';
      icon = <ConnectedIcon width="20" />;
      break;
    case LoadStatusType.outdated:
      title = 'Environment readings are outdated.';
      icon = <OutdatedIcon width="20" />;
      break;
    case LoadStatusType.disconnected:
      title = 'Disconnected.';
      icon = <DisconnectedIcon width="20" />;
      break;
    default:
      throw Error(`Unknown status type ${status}`);
  }

  return <div title={title}>{icon}</div>;
}

LoadStatus.propTypes = propTypes;

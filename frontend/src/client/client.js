import React from 'react';
import ReactDOM from 'react-dom';

import 'utils/register-service-worker';
import startSyncTimeWithServer from 'utils/sync-time-with-server';
import App from './components/App';

(async () => {
  await startSyncTimeWithServer();

  ReactDOM.render(<App />, document.querySelector('#root'));
})();

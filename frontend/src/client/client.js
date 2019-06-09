import React from 'react';
import ReactDOM from 'react-dom';

import 'utils/register-service-worker';
import App from './components/App';

ReactDOM.render(<App />, document.querySelector('#root'));

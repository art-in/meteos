import React, {useState} from 'react';

import classes from './App.css';

export default function App() {
  const [count, setCount] = useState(0);

  return (
    <div className={classes.root}>
      <p>You clicked {count} times</p>
      <button onClick={() => setCount(count + 1)}>Click me</button>
    </div>
  );
}

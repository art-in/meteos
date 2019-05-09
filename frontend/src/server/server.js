'use strict';

const express = require('express');
const path = require('path');
const logger = require('morgan');

const PORT = 3000;
const HOST = '0.0.0.0';

const app = express();

app.use(logger('dev'));
app.use(express.static(path.resolve(__dirname, '../client/dist/')));

app.listen(PORT, HOST);
console.log(`Listening on http://${HOST}:${PORT}`);

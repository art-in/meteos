'use strict';

const express = require('express');
const path = require('path');
const logger = require('morgan');
const proxy = require('http-proxy-middleware');
const yargs = require('yargs');

const PORT = 3001;
const HOST = '0.0.0.0';

const argv = yargs
  .version(false)
  .help(false)
  .option('backend-url', {
    type: 'string',
    demandOption: true
  }).argv;

const app = express();

app.use(logger('dev'));
app.use(express.static(path.resolve(__dirname, '../client/dist/')));
app.use('/api', proxy({target: argv.backendUrl, pathRewrite: {'^/api': ''}}));

app.listen(PORT, HOST);
console.log(`Backend URL: ${argv.backendUrl}`);
console.log(`Listening on http://${HOST}:${PORT}`);

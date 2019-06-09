'use strict';

const http = require('http');
const https = require('https');
const fs = require('fs');

const express = require('express');
const path = require('path');
const logger = require('morgan');
const proxy = require('http-proxy-middleware');
const yargs = require('yargs');
const table = require('text-table');

const PORT = 3001;
const HOST = '0.0.0.0';
const CERT_FOLDER = '/opt/cert/';

const argv = yargs
  .version(false)
  .help(false)
  .option('backend-url', {
    type: 'string',
    demandOption: true
  })
  .option('tls-key', {type: 'string'})
  .option('tls-cert', {type: 'string'}).argv;

const app = express();

app.use(logger('dev'));
app.use(express.static(path.resolve(__dirname, '../client/dist/')));
app.use('/api', proxy({target: argv.backendUrl, pathRewrite: {'^/api': ''}}));

const isTLS = argv.tlsKey && argv.tlsCert;

let server;
if (isTLS) {
  server = https.createServer(
    {
      key: fs.readFileSync(path.resolve(CERT_FOLDER, argv.tlsKey)),
      cert: fs.readFileSync(path.resolve(CERT_FOLDER, argv.tlsCert))
    },
    app
  );
} else {
  server = http.createServer(app);
}

server.listen(PORT, HOST, function(err) {
  if (err) {
    throw Error(err);
  }

  const scheme = isTLS ? 'https' : 'http';

  console.log(
    `Server started\n` +
      table([
        ['\t backend url', argv.backendUrl],
        ['\t tls key file', isTLS ? argv.tlsKey : '---'],
        ['\t tls cert file', isTLS ? argv.tlsCert : '---'],
        ['\t listening at', `${scheme}://${HOST}:${PORT}`]
      ])
  );
});

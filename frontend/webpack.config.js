const path = require('path');

module.exports = (env, argv) => ({
  entry: path.resolve(__dirname, 'src/client/client.js'),
  output: {
    path: path.resolve(__dirname, 'src/client/dist'),
    filename: 'bundle.js'
  },
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader'
        }
      }
    ]
  },
  devtool: argv.mode == 'development' ? 'inline-source-map' : false,
  devServer: {
    // URL that dev-server will listen on.
    host: '0.0.0.0',
    port: 8080,

    // requests to this URL path will be served by dev-server (see proxy)
    publicPath: '/',

    // filename of bundle that will be served by dev-server from memory
    filename: 'bundle.js',

    // file system path to serve static files from (see proxy)
    contentBase: path.resolve(__dirname, 'src/client/dist'),

    proxy: {
      // everything except bundle should be served by server
      ['!bundle.js']: {target: 'http://localhost:3000', secure: false}
    },

    // watch files in contentBase and reload page on changes.
    watchContentBase: true,
    watchOptions: {
      // detect file changes by polling (otherwise does not watch in container)
      poll: true
    },

    // do not spam webpack cli output
    stats: 'minimal',

    // do not spam browser log
    clientLogLevel: 'warning'
  }
});

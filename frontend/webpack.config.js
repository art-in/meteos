const path = require('path');
const webpack = require('webpack');
const WorkboxPlugin = require('workbox-webpack-plugin');

module.exports = (env, argv) => ({
  entry: path.resolve(__dirname, 'src/client/client.js'),
  output: {
    path: path.resolve(__dirname, 'src/client/dist'),
    filename: 'bundle.js'
  },
  plugins: [
    argv.mode == 'production' &&
      new WorkboxPlugin.GenerateSW({
        // do not use CDN for workbox runtime libs
        importWorkboxFrom: 'local',
        swDest: 'sw-cache.js',
        importsDirectory: 'sw-cache-assets',

        // add more files to cache
        globDirectory: path.resolve(__dirname, 'src/client/dist'),
        globPatterns: [
          'index.html',
          'manifest.json',
          'favicon-16.png',
          'favicon-128.png'
        ]
      }),
    
    // ignore all locale files of moment.js
    new webpack.IgnorePlugin(/^\.\/locale$/, /moment$/),

    // uncomment to analyze bundle
    // new (require('webpack-bundle-analyzer')).BundleAnalyzerPlugin({
    //   analyzerMode: 'static',
    //   reportFilename: 'bundle-analyzer-report.html'
    // })
  ].filter(p => p),
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader'
        }
      },
      {
        test: /\.css$/,
        use: [
          {
            loader: 'style-loader'
          },
          {
            loader: 'css-loader',
            options: {
              modules: true,
              camelCase: true,
              localIdentName: '[name]-[local]'
            }
          },
          {
            loader: 'postcss-loader'
          }
        ]
      },
      {
        test: /\.(ttf|otf|eot|woff|woff2)$/,
        use: [
          {
            loader: 'url-loader',
            options: {
              // add to bundle in form of base64 data url
              // only if file size is less than limit
              limit: 8192
            }
          }
        ]
      },
      {
        test: /\.svg$/,
        use: ['@svgr/webpack']
      }
    ]
  },
  resolve: {
    modules: [path.resolve(__dirname, 'src/client'), 'node_modules']
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
      ['!bundle.js']: {target: 'http://localhost:3001', secure: false}
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

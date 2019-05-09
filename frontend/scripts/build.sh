#!/bin/bash

# builds project

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# build client
npx webpack --config $(pwd)/webpack.config.js --mode=production --progress
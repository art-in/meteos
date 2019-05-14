#!/bin/bash

# kill previous instances to release the port on restarts
ps -ef | grep 'src/mock/backend.js' | grep -v grep | awk '{print $2}' | xargs -r kill -9

npx nodemon --legacy-watch --watch $(pwd)/src/mock $(pwd)/src/mock/backend.js
#!/bin/bash

# runs project
#
# options:
# --dev - development mode (rebuild on src/ changes, skip optimizations, etc.)

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# handle args
while test $# -gt 0
do
    case "$1" in
        --dev) ARG_DEV=1 ;;
        # sleep infinity instead of just exit, because when container starts with
        # vs-code 'open folder in container' entry point with this script is not 
        # getting overriden with "sleep infinity" as it should.
        # https://github.com/microsoft/vscode-remote-release/issues/259
        *) echo "Invalid argument '$1'." && sleep infinity ;; # TODO: exit 1
    esac
    shift
done

if [[ $ARG_DEV ]] ; then
    npx webpack-dev-server --config $(pwd)/webpack.config.js --stdin=false --mode=development &
    npx nodemon --legacy-watch --watch $(pwd)/src/server $(pwd)/src/server/server.js
else
    node $(pwd)/src/server/server.js
fi
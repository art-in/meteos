#!/bin/bash

# runs frontend server
#
# options:
# --backend-url - (required) url of backend service to fetch environment data from
# --dev         - development mode (rebuild on src/ changes, skip optimizations, etc.)
# --tls-key     - file name of certificate private key 
# --tls-cert    - file name of certificate

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# handle args
while test $# -gt 0
do
    case "$1" in
        --dev) ARG_DEV=1 ;;
        --backend-url*) ARG_BACKEND_URL=$1 ;;
        --tls-key*) ARG_TLS_KEY=$1 ;;
        --tls-cert*) ARG_TLS_CERT=$1 ;;
        # sleep infinity instead of just exit, because when container starts with
        # vs-code 'open folder in container' entry point with this script is not 
        # getting overriden with "sleep infinity" as it should.
        # https://github.com/microsoft/vscode-remote-release/issues/259
        *) echo "Invalid argument '$1'." && sleep infinity ;; # TODO: exit 1
    esac
    shift
done

if [[ $ARG_DEV ]] ; then
    # kill previous instances to release the port on restarts
    ps -ef | grep '/src/server/server.js' | grep -v grep | awk '{print $2}' | xargs -r kill -9

    npx webpack-dev-server --config $(pwd)/webpack.config.js --stdin=false --mode=development &
    NODE_ENV=development npx nodemon --legacy-watch --watch $(pwd)/src/server $(pwd)/src/server/server.js $ARG_BACKEND_URL
else
    mkdir -p /opt/log
    node $(pwd)/src/server/server.js $ARG_BACKEND_URL $ARG_TLS_KEY $ARG_TLS_CERT
fi
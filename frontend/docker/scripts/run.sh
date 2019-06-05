#!/bin/bash

# runs built docker image.
#
# options:
# --backend-url - (required) url of backend service to fetch environment data from
# --dev         - development mode (rebuild on src/ changes, skip optimizations, etc.)
# --tls-folder  - folder with tls certificate
# --tls-key     - file name of certificate private key 
# --tls-cert    - file name of certificate

PROJECT_NAME=meteos-frontend
DOCKER_IMAGE=$PROJECT_NAME

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

# handle args
while test $# -gt 0
do
    case "$1" in
        --dev) ARG_DEV=1 ;;
        --backend-url*) ARG_BACKEND_URL=$1 ;;
        --tls-folder*) ARG_TLS_FOLDER=$1 ;;
        --tls-key*) ARG_TLS_KEY=$1 ;;
        --tls-cert*) ARG_TLS_CERT=$1 ;;
        *) echo "Invalid argument '$1'." && exit 1 ;;
    esac
    shift
done

if [[ -z $ARG_BACKEND_URL ]] ; then
    echo "Missing required argument: --backend-url"; exit 1;
fi

if [[ $ARG_TLS_FOLDER ]] ; then
    if [[ $ARG_DEV ]] ; then
        echo "TLS is not supported in dev mode"; exit 1;
    fi

    PARTS=(${ARG_TLS_FOLDER//=/ }) # split by '='
    TLS_FOLDER=${PARTS[1]}
    ARG_TLS_FOLDER_MOUNT="--mount type=bind,src=$TLS_FOLDER,dst=/opt/cert"
fi

# stop other containers with this image to release the port
docker rm $(docker stop $(docker ps -a -q --filter ancestor=$DOCKER_IMAGE --format="{{.ID}}"))

# run
echo "Running docker container '$DOCKER_IMAGE'..." ; echo ""
if [[ $ARG_DEV ]] ; then
    docker run \
        --mount type=bind,src=$(pwd)/src/,dst=/opt/project/src/ \
        -p 3001:3001 \
        -p 8080:8080 \
        -i \
        $DOCKER_IMAGE \
        $ARG_BACKEND_URL \
        --dev
else
    docker run \
        $ARG_TLS_FOLDER_MOUNT \
        -p 3001:3001  \
        $DOCKER_IMAGE \
        $ARG_BACKEND_URL \
        $ARG_TLS_KEY \
        $ARG_TLS_CERT
fi
#!/bin/bash

# runs built docker image.
#
# options:
# --dev - development mode (rebuild on src/ changes, skip optimizations, etc.)

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
        *) echo "Invalid argument '$1'." && exit 1 ;;
    esac
    shift
done

if [[ -z $ARG_BACKEND_URL ]] ; then
    echo "Missing required argument: --backend-url"; exit 1;
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
        -p 3001:3001  \
        $DOCKER_IMAGE \
        $ARG_BACKEND_URL
fi
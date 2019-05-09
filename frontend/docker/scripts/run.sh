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
        *) echo "Invalid argument '$1'." && exit 1 ;;
    esac
    shift
done

# stop other containers with this image to release the port
docker rm $(docker stop $(docker ps -a -q --filter ancestor=$DOCKER_IMAGE --format="{{.ID}}"))

# run
echo "Running docker container '$DOCKER_IMAGE'..." ; echo ""
if [[ $ARG_DEV ]] ; then
    docker run \
        --mount type=bind,src=$(pwd)/src/,dst=/opt/project/src/ \
        -p 3000:8080  \
        -i \
        $DOCKER_IMAGE \
        --dev
else
    docker run \
        -p 3000:3000  \
        $DOCKER_IMAGE
fi
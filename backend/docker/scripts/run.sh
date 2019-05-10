#!/bin/bash

# runs built docker image.
#
# options:
# --test - runs tests inside container.

PROJECT_NAME=meteos-backend
DOCKER_IMAGE=$PROJECT_NAME

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

# handle args
while test $# -gt 0
do
    case "$1" in
        --test) TEST=1 ;;
	*) echo "Invalid argument '$1'." && exit 1 ;;
    esac
    shift
done

# run
echo "Running docker container '$DOCKER_IMAGE'..." ; echo ""
if [[ $TEST ]] ; then
    docker run $DOCKER_IMAGE --test
else
    mkdir -p ./build/data
    docker run \
        --mount type=bind,src=$(pwd)/build/data/,dst=/opt/project/build/bin/data/ \
        -p 3000:3000  \
        -i \
        $DOCKER_IMAGE
fi

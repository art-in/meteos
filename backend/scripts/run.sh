#!/bin/bash

# runs built docker image.
# options:
# --test - runs tests inside container.

PROJECT_NAME=meteos-backend
DOCKER_IMAGE=$PROJECT_NAME

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# handle args
if [[ "$1" == "--test" ]] ; then
    TEST=1
fi

# run
echo "Running docker container '$PROJECT_NAME'..." ; echo ""
if [[ $TEST ]] ; then
    docker run $DOCKER_IMAGE --test
else
    mkdir -p ./build/data
    docker run \
        --mount type=bind,src=$(pwd)/build/data/,dst=/opt/project/build/bin/data/ \
        -p 8080:8080  \
        -i \
        $DOCKER_IMAGE
fi

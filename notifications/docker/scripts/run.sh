#!/bin/bash

# runs docker image

PROJECT_NAME=meteos-notifications
DOCKER_IMAGE=$PROJECT_NAME
DOCKER_CONTAINER=$PROJECT_NAME

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

# stop other containers with this image
docker container rm -f $DOCKER_CONTAINER

# run
docker run \
    --detach \
    --interactive \
    --name $DOCKER_CONTAINER \
    $DOCKER_IMAGE

#!/bin/bash

# creates docker image with necessary tools and builds project inside of it.
#
# options:
# --clean - recreates docker image skipping cache.
#           use it when changing build tools or project dependencies.

PROJECT_NAME=meteos-frontend
DOCKER_IMAGE=$PROJECT_NAME

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

# handle args
while test $# -gt 0
do
    case "$1" in
        --clean) ARG_CLEAN=1 ;;
        *) echo "Invalid argument '$1'." && exit 1 ;;
    esac
    shift
done

if [[ $ARG_CLEAN ]] ; then
    DOCKER_ARG_NOCACHE=--no-cache
fi

# create docker image
echo "Creating docker image '$DOCKER_IMAGE'..."
docker build . -f ./docker/Dockerfile -t $DOCKER_IMAGE $DOCKER_ARG_NOCACHE
echo "Created docker image '$DOCKER_IMAGE'."
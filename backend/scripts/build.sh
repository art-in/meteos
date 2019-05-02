#!/bin/bash

# creates docker image with necessary tools and builds project inside it.
# options:
# --tools - recreates docker image with build tools even if it already exists.
#           use it when changing build tools or project dependencies.
# --test  - additionaly builds tests.

PROJECT_NAME=meteos-backend

DOCKER_IMAGE_BUILD=$PROJECT_NAME
DOCKER_IMAGE_TOOLS=$PROJECT_NAME-tools

DOCKER_IMAGE_TOOLS_EXISTS=$(docker image ls | grep $DOCKER_IMAGE_TOOLS)

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# handle args
while test $# -gt 0
do
    case "$1" in
        --tools) ARG_TOOLS=1 ;;
        --test) ARG_TEST=1 ;;
    esac
    shift
done

if [ $ARG_TEST ] ; then
    DOCKER_ARG_TEST="--build-arg BUILD_TEST=ON"
fi

# create docker image with build tools if not exists yet
if [ -z "$DOCKER_IMAGE_TOOLS_EXISTS" ] || [ $ARG_TOOLS ] ; then
    echo "Creating docker image '$DOCKER_IMAGE_TOOLS'..."
    docker build . --no-cache -f ./docker/Dockerfile-tools -t $DOCKER_IMAGE_TOOLS
else
    echo "Skip creating docker image '$DOCKER_IMAGE_TOOLS' as it already exists."
fi

# create docker image for build
echo "Creating docker image '$DOCKER_IMAGE_BUILD'..."
docker image rm --force $DOCKER_IMAGE_BUILD
docker build . --no-cache -f ./docker/Dockerfile -t $DOCKER_IMAGE_BUILD $DOCKER_ARG_TEST
echo "Created docker image '$DOCKER_IMAGE_BUILD'."
#!/bin/bash

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

./docker/scripts/build.sh --test
./docker/scripts/run.sh --test
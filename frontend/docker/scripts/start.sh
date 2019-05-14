#!/bin/bash

# go to project root
cd $(dirname $(realpath "$0")) && cd ../..

./docker/scripts/build.sh
./docker/scripts/run.sh $1 $2 # pass arguments through

#!/bin/bash

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

./scripts/build.sh --test
./scripts/run.sh --test
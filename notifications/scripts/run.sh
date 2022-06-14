#!/bin/bash

# runs notification service

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# run
target/release/meteos-notifications 
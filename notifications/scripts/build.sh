#!/bin/bash

# builds notification service

# go to project root
cd $(dirname $(realpath "$0")) && cd ..

# build
cargo build --release
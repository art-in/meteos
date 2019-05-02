#!/bin/bash

# runs built binary inside docker container.

if [[ "$1" == "--test" ]] ; then
    # run service
    # service will exit if cin is closed, so keep it open while running in background
    mkfifo in
    tail -f in | ./bin/meteos-backend &

    # wait a bit for service to start
    sleep 0.5s

    # run tests
    ./bin/meteos-backend-test --use-colour yes
else
    ./bin/meteos-backend
fi


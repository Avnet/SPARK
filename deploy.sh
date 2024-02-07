#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <target>"
    exit 1
fi

TARGET=$1

(
    set -ex

    scp -r ./app/exe/ "root@${TARGET}:/home/root/dev/"

    set +ex
)

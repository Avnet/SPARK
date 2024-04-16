#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <target>"
    echo "Dont forget to enable your iotc-spark-server.service service"
    exit 1
fi

TARGET=$1

(
    set -ex

    ssh "root@${TARGET}" "mkdir -p /home/root/dev/; mkdir -p /opt/spark/iot/"
    scp -r ./app/exe/ "root@${TARGET}:/home/root/dev/"
    scp -r ./utils/ "root@${TARGET}:/home/root/dev/"
    scp ./app/iot/*.json "${TARGET}:/opt/spark/iot/"
    scp ./app/iot/iotc-spark-server.service "${TARGET}:/etc/systemd/system/"

    set +ex
)

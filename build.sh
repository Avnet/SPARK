#!/bin/bash

(
    set -e
    cd ./app/src
    mkdir -p build && cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake ..
    make -j"$(nproc)"
    set +e
)

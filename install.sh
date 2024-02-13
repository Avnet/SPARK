#!/bin/bash

# Install SPARK into RZBoard
INSTALL_DIR=/opt/spark
mkdir -p "${INSTALL_DIR}"

cp -r ./app/exe/* "${INSTALL_DIR}"

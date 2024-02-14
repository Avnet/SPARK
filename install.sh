#!/bin/bash

# Install SPARK into RZBoard
INSTALL_DIR=/opt/spark
mkdir -p "${INSTALL_DIR}"

cp -r ./app/exe/* "${INSTALL_DIR}"
cp ./runtime_deps/libtvm_runtime.so /usr/lib64/
cat ./app/weston_conf/weston.ini.append >> /etc/xdg/weston/weston.ini

echo "SPARK installed successfully. Restarting Weston for changes to take effect."

systemctl restart weston@root

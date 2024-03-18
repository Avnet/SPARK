#!/bin/bash

# Install SPARK into RZBoard
INSTALL_DIR=/opt/spark
mkdir -p "${INSTALL_DIR}"

cp -r ./app/exe/* "${INSTALL_DIR}" || { echo "Failed to copy SPARK to RZBoard"; exit 1;}
cp ./runtime_deps/libtvm_runtime.so /usr/lib64/ || { echo "Failed to copy TVM runtime to RZboard"; exit 1;}
cat ./app/weston_conf/weston.ini.append >> /etc/xdg/weston/weston.ini || { echo "Failed to append weston.ini"; exit 1;}

echo "SPARK installed successfully. Restarting Weston for changes to take effect."

systemctl restart weston@root || { echo "Failed to restart Weston"; exit 1;}

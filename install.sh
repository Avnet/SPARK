#!/bin/bash

# Install SPARK into RZBoard
INSTALL_DIR=/opt/spark
mkdir -p "${INSTALL_DIR}"

cp -r ./app/exe/* "${INSTALL_DIR}" || { echo "Failed to copy SPARK to RZBoard"; exit 1;}
cp ./runtime_deps/libtvm_runtime.so /usr/lib64/ || { echo "Failed to copy TVM runtime to RZboard"; exit 1;}
cat ./app/weston_conf/weston.ini.append >> /etc/xdg/weston/weston.ini || { echo "Failed to append weston.ini"; exit 1;}

# IoT Config
mkdir -p ${INSTALL_DIR}/iot || { echo "Failed to create iot directory"; exit 1;}
cp ./app/iot/*.json ${INSTALL_DIR}/iot/ || { echo "Failed to copy iot json files"; exit 1;}
echo "Don't forget to update /opt/spark/iot/ config files with your own device keys/server orientation"

# Service management
cp ./app/iot/iotc-spark-server.service /etc/systemd/system/ || { echo "Failed to copy iotc-spark-server.service"; exit 1;}
systemctl daemon-reload
systemctl enable iotc-spark-server.service || { echo "Failed to enable iotc-spark-server.service"; exit 1;}
systemctl start iotc-spark-server.service || { echo "Failed to start iotc-spark-server.service"; exit 1;}

echo "SPARK installed successfully. Restarting Weston for changes to take effect."

systemctl restart weston@root || { echo "Failed to restart Weston"; exit 1;}

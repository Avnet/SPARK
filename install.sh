#!/bin/bash

# Install SPARK into RZBoard
INSTALL_DIR=/opt/spark
mkdir -p "${INSTALL_DIR}"

echo "Installing SPARK executables, utils, and libs into RZBoard... The executables will be copied to ${INSTALL_DIR} directory."
cp -r ./app/exe/* "${INSTALL_DIR}" || { echo "Failed to copy SPARK to RZBoard"; exit 1;}
cp -r ./utils/* "${INSTALL_DIR}" || { echo "Failed to copy SPARK utils to RZBoard";}
cp ./runtime_deps/libtvm_runtime.so /usr/lib64/ || { echo "Failed to copy TVM runtime to RZboard"; exit 1;}
cat ./app/weston_conf/weston.ini.append >> /etc/xdg/weston/weston.ini || { echo "Failed to append weston.ini. You will have to run spark from ${INSTALL_DIR}";}
echo "Done installing SPARK executables, utils, and libs into RZBoard..."

# IoT Config
echo "Installing IoT Connect configurations into RZBoard..."
mkdir -p ${INSTALL_DIR}/iot || { echo "Failed to create iot directory"; exit 1;}
cp ./app/iot/*.json ${INSTALL_DIR}/iot/ || { echo "Failed to copy iot json files"; exit 1;}
echo "Don't forget to update /opt/spark/iot/ config files with your own device keys/server orientation"
echo "Done installing IoT Connect configurations into RZBoard..."

# Service management
echo "Installing IoT Connect / SPARK Service into RZBoard... The service will be enabled (auto launches on internet connection) and started after installation."
cp ./app/iot/iotc-spark-server.service /etc/systemd/system/ || { echo "Failed to copy iotc-spark-server.service"; exit 1;}
systemctl daemon-reload
systemctl enable iotc-spark-server.service || { echo "Failed to enable iotc-spark-server.service"; exit 1;}
systemctl start iotc-spark-server.service || { echo "Failed to start iotc-spark-server.service"; exit 1;}
echo "Done installing IoT Connect / SPARK Service into RZBoard... The service is enabled and started after installation."

echo "SPARK installed successfully. Restarting Weston for changes to take effect."

systemctl restart weston@root || { echo "Failed to restart Weston"; exit 1;}

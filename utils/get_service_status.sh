echo "Checking iotc-spark-server.service status..."
systemctl list-units --type=service --all | grep -i iotc-spark-server.service

echo "Checking iotc-spark-server.service logs..."
journalctl -u iotc-spark-server.service

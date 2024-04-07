echo "Printing SPARK iotc related files at /opt/spark/iot/"
echo "------------------------------------------"
find /opt/spark/iot/ -type f
echo "------------------------------------------"

echo "IoT service secrets.json file content"
echo "------------------------------------------"
cat /opt/spark/iot/secrets.json
if [ $? -ne 0 ]; then
    echo "secrets.json file not found. Ensure it exists in /opt/spark/iot/ directory"
fi
echo "------------------------------------------"

echo "Please cross reference the files and secrets.json to ensure all dependencies exist"

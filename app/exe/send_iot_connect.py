"""SPARK & IoTConnect Integration"""

import json
import sys
import time
import random
from datetime import datetime
from typing import Dict, Any

from iotconnect import IoTConnectSDK

class IoTConnectClient:
    """Send SPARK data to IoTConnect platform using IoTConnect SDK."""
    def __init__(self, config_path: str) -> None:
        self.load_config(config_path)
        self.sdk_options = {
            "certificate": {
                "SSLKeyPath": self.config['ssl']['keyPath'],
                "SSLCertPath": self.config['ssl']['certPath'],
                "SSLCaPath": self.config['ssl']['caPath'],
            },
            "offlineStorage": {
                "disabled": False,
                "availSpaceInMb": 0.01,
                "fileCount": 5,
                "keepalive": 60
            },
            "skipValidation": False,
            "discoveryUrl": self.config['networking']['discoveryUrl'],
            "IsDebug": True,
            "transmit_interval_seconds": 15,
        }
        self.sdk = None
        self.device_list = []

    def load_config(self, config_path: str) -> None:
        """Dependency inject client configuration"""
        try:
            with open(config_path, encoding="utf-8") as f:
                self.config = json.load(f)
        except Exception as e:
            print(e)
            sys.exit(1)

    def device_callback(self, msg: Dict[str, Any]) -> None:
        print("\n--- Command Message Received in Firmware ---")
        print("--- No further business logic implemented ---")
        print(json.dumps(msg))

    def device_firmware_callback(self, msg: Dict[str, Any]) -> None:
        print("\n--- Firmware Command Message Received ---")
        print("--- No further business logic implemented ---")
        print(json.dumps(msg))

    def device_connection_callback(self, msg: Dict[str, Any]) -> None:
        print("\n--- Device coonnection callback: no further business logic implemented ---")
        print(json.dumps(msg))

    def twin_update_callback(self, msg: Dict[str, Any]) -> None:
        print("--- Twin Message Received ---")
        print("--- No further business logic implemented ---")
        print(json.dumps(msg))

    def send_data(self) -> None:
        payload = [{
            "uniqueId": self.config['ids']['uniqueId'],
            "time": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%S.000Z"),
            "data": {
                "empty": random.randint(2, 8),
                "taken": random.randint(1, 6),
                "location": [49, 11]
            },
        }]
        print(f"Sending payload: {payload}")
        self.sdk.SendData(payload)

    def run_telemetry_continuously(self) -> None:
        try:
            with IoTConnectSDK(self.config['ids']['uniqueId'], self.config['ids']['sid'], self.sdk_options, self.device_connection_callback) as self.sdk:
                self.device_list = self.sdk.Getdevice()
                self.sdk.onDeviceCommand(self.device_callback)
                self.sdk.onTwinChangeCommand(self.twin_update_callback)
                self.sdk.onOTACommand(self.device_firmware_callback)
                self.sdk.onDeviceChangeCommand(self.device_connection_callback)
                self.sdk.getTwins()
                self.device_list = self.sdk.Getdevice()

                while True:
                    self.send_data()
                    time.sleep(self.sdk_options['transmit_interval_seconds'])

        except KeyboardInterrupt:
            print("Keyboard Interrupt, exiting")
            sys.exit(0)
        except Exception as ex:
            print(ex)
            sys.exit(1)

if __name__ == "__main__":
    client = IoTConnectClient('/opt/SPARK/iot/secrets.json')
    client.run_telemetry_continuously()

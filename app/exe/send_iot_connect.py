"""SPARK & IoTConnect Integration"""

import json
import socket
import sys
import time
import random
from datetime import datetime
from typing import Dict, Any, List

from iotconnect import IoTConnectSDK

class IoTConnectClient:
    """Send SPARK data to IoTConnect platform using IoTConnect SDK."""
    def __init__(self, config_path: str) -> None:
        self.config = {}
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

    def load_config(self, config_paths: List[str]) -> None:
        """Dependency inject client configuration"""
        try:
            for config_path in config_paths:
                with open(config_path, encoding="utf-8") as f:
                    # Load configuration from JSON file(s) with UNIQUE KEYS
                    self.config.update(json.load(f))
            print(f"Configuration loaded successfully {self.config}")
        except Exception as e:
            print(e)
            sys.exit(1)

    def connect_spark_socket(self, retry_interval: int = 5) -> socket.socket:
        """Attempt connection (continuously) to SPARK producer socket"""
        while True:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((self.config['spark_socket_ipv4'], self.config['spark_socket_port']))
                print("Connected to SPARK producer socket")
                return sock
            except ConnectionRefusedError:
                print("Connection refused, retrying...")
            except Exception as e:
                print(f"An error occurred: {e}. Retrying...")
            finally:
                time.sleep(retry_interval)

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
                socket = self.connect_spark_socket()
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
    client = IoTConnectClient(['/opt/SPARK/iot/secrets.json', '/opt/SPARK/iot/config.json'])
    client.run_telemetry_continuously()

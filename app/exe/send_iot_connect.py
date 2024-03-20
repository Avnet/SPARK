"""SPARK & IoTConnect Integration"""

import json
import socket
import sys
import signal
import time
import random
from datetime import datetime
from typing import Dict, Any, List

from iotconnect import IoTConnectSDK

class SignalException(Exception):
    """Custom exception to exit gracefully"""

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
        self.setup_exit_handler()
        self.run_continuously = True

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

    def get_spark_datagram_socket(self) -> socket.socket:
        """Attempt connection (continuously) to SPARK producer socket"""
        max_retry_backoff_s = 8
        retry_backoff_s = 1
        sock = None
        while self.run_continuously:
            try:
                for addr in socket.getaddrinfo(self.config['spark_socket_ipv6'], self.config['spark_socket_port'], socket.AF_INET6, socket.SOCK_DGRAM, 0, socket.AI_PASSIVE):
                    af, socktype, proto, _, sa = addr
                    try:
                        sock = socket.socket(af, socktype, proto)
                        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    except OSError as msg:
                        print(f"Socket init failed: {msg}. Retrying...")
                        sock = None
                        continue
                    try:
                        sock.bind(sa)
                    except OSError as msg:
                        print(f"Socket bind failed: {msg}. Retrying...")
                        sock.close()
                        sock = None
                        continue
                    # Success
                    break

                if sock is None:
                    raise ConnectionError("Failed to create socket")

                return sock
            except SignalException:
                sys.exit(0)
            except Exception as msg:
                retry_backoff_s = min(max_retry_backoff_s, retry_backoff_s * 2)
                time.sleep(retry_backoff_s)


    def setup_exit_handler(self) -> None:
        """Define exit conditions as application will typically run indefinitely"""
        signal.signal(signal.SIGINT, self.exit_handler)
        signal.signal(signal.SIGTERM, self.exit_handler)

    def exit_handler(self, _sig, _frame) -> None:
        """Stop IoT/SPARK integration"""
        print("Exit signal detected. Exiting...")
        self.run_continuously = False
        raise SignalException("Exit signal detected")

    
    def receive_taken_empty_spark_data(self, sock: socket.socket) -> (int, int):
        """Receive SPARK data from producer socket"""
        # return (taken, empty)
        # Data comes over the wire as bytes('taken,empty\n') where taken, empty are integers
        if sock is None:
            raise ConnectionError("SPARK producer socket not connected")

        # return random.randint(0, 100), random.randint(0, 100)
        bytes_data, _addr = sock.recvfrom(1024)
        if not bytes_data:
            raise ConnectionError("SPARK producer socket closing")
        
        # Assuming the c++ std::string is utf-8 compatible...
        utilizations = bytes_data.decode('utf-8').split('\n')[0]
        print(f"Received data: {utilizations}")
        util_list = utilizations.split(',')
        return tuple(map(int, util_list))

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

    def send_data(self, empty, taken) -> None:
        payload = [{
            "uniqueId": self.config['ids']['uniqueId'],
            "time": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%S.000Z"),
            "data": {
                "empty": empty,
                "taken": taken,
                "location": [49, 11]
            },
        }]
        print(f"Sending payload: {payload}")
        self.sdk.SendData(payload)

    def run_telemetry_continuously(self) -> None:
        """Run IoTConnect client continuously"""
        max_retry_backoff_s = 300
        retry_backoff_s = 5
        while self.run_continuously:
            try:
                with IoTConnectSDK(self.config['ids']['uniqueId'], self.config['ids']['sid'], self.sdk_options, self.device_connection_callback) as self.sdk:
                    self.device_list = self.sdk.Getdevice()
                    self.sdk.onDeviceCommand(self.device_callback)
                    self.sdk.onTwinChangeCommand(self.twin_update_callback)
                    self.sdk.onOTACommand(self.device_firmware_callback)
                    self.sdk.onDeviceChangeCommand(self.device_connection_callback)
                    self.sdk.getTwins()
                    self.device_list = self.sdk.Getdevice()
                    spark_socket = self.get_spark_datagram_socket()
                    while True:
                        empty, taken = self.receive_taken_empty_spark_data(spark_socket)
                        self.send_data(empty, taken)
                        time.sleep(self.sdk_options['transmit_interval_seconds'])
            except SignalException:
                sys.exit(0)
            # exponential backoff
            except Exception as ex:
                print(ex)
                time.sleep(retry_backoff_s)
                retry_backoff_s = min(max_retry_backoff_s, retry_backoff_s * 2)

if __name__ == "__main__":
    client = IoTConnectClient(['/opt/spark/iot/secrets.json', '/opt/spark/iot/config.json'])
    client.run_telemetry_continuously()

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
        print(f"Configuration loaded: {self.config}")
        # TODO: Put more of these options in the configuration file
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
            "transmit_interval_seconds": 5
        }
        self.sdk = None
        self.device_list = []
        self.setup_exit_handler()
        self.run_continuously = True
        print("IoTConnect service initialized")

    def load_config(self, config_paths: List[str]) -> None:
        """Dependency inject client configuration"""
        try:
            for config_path in config_paths:
                with open(config_path, encoding="utf-8") as f:
                    # Load configuration from JSON file(s) with UNIQUE KEYS
                    self.config.update(json.load(f))
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
        print("\n--- Device Callback ---")
        print(json.dumps(msg))
        cmd_type = None
        if msg is not None and len(msg.items()) != 0:
            cmd_type = msg["ct"] if "ct"in msg else None
        # Other Command
        if cmd_type == 0:
            """
            * Type    : Public Method "sendAck()"
            * Usage   : Send device command received acknowledgment to cloud
            * 
            * - status Type
            *     st = 6; // Device command Ack status 
            *     st = 4; // Failed Ack
            * - Message Type
            *     msgType = 5; // for "0x01" device command 
            """
            data=msg
            if data is None:
                #print(data)
                if "id" in data:
                    if "ack" in data and data["ack"]:
                        self.sdk.sendAckCmd(data["ack"],7,"sucessfull",data["id"])  #fail=4,executed= 5,sucess=7,6=executedack
                else:
                    if "ack" in data and data["ack"]:
                        self.sdk.sendAckCmd(data["ack"],7,"sucessfull") #fail=4,executed= 5,sucess=7,6=executedack
        else:
            print("rule command",msg)
        

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
                print("Starting IoTConnect SDK")
                with IoTConnectSDK(self.config['ids']['uniqueId'], self.config['ids']['sid'], self.sdk_options, self.device_connection_callback) as self.sdk:
                    self.device_list = self.sdk.Getdevice()
                    self.sdk.onDeviceCommand(self.device_callback)
                    self.sdk.onTwinChangeCommand(self.twin_update_callback)
                    self.sdk.onOTACommand(self.device_firmware_callback)
                    self.sdk.onDeviceChangeCommand(self.device_connection_callback)
                    self.sdk.getTwins()
                    self.device_list = self.sdk.Getdevice()
                    spark_socket = self.get_spark_datagram_socket()
                    next_transmit = time.time()
                    print("Sending telemetry data to IoTConnect")
                    while True:
                        taken, empty = self.receive_taken_empty_spark_data(spark_socket)
                        if time.time() > next_transmit:
                            self.send_data(empty, taken)
                            next_transmit = time.time() + self.sdk_options['transmit_interval_seconds']
            except SignalException:
                sys.exit(0)
            # exponential backoff
            except Exception as ex:
                print(f'Backing off from {ex}')
                time.sleep(retry_backoff_s)
                retry_backoff_s = min(max_retry_backoff_s, retry_backoff_s * 2)

if __name__ == "__main__":
    client = IoTConnectClient(['/opt/spark/iot/secrets.json', '/opt/spark/iot/config.json'])
    client.run_telemetry_continuously()

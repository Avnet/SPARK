"Send SPARK data to IoTConnect platform using IoTConnect SDK"

import sys
import json
import time
import random
from datetime import datetime

from iotconnect import IoTConnectSDK

"""
* ## Prerequisite parameters
* cpId         :: Need to get from the IoTConnect platform "Settings->Key Vault". 
* uniqueId     :: Its device ID which register on IotConnect platform and also its status has Active and Acquired
* env          :: Need to get from the IoTConnect platform "Settings->Key Vault". 
* interval     :: send data frequency in seconds
* sdkOptions   :: It helps to define the path of self signed and CA signed certificate as well as define the offlinne storage configuration.
"""

UniqueId = None
SId = None
Sdk = None
interval = 30
directmethodlist = {}
ACKdirect = []
device_list = []

SdkOptions = {
    "certificate": {
        "SSLKeyPath": "",  # aws=pk_devicename.pem   ||   #az=device.key
        "SSLCertPath": "",  # aws=cert_devicename.crt ||   #az=device.pem
        "SSLCaPath": "",  # aws=root-CA.pem         ||   #az=rootCA.pem
    },
    "offlineStorage": {"disabled": False, "availSpaceInMb": 0.01, "fileCount": 5, "keepalive": 60},
    "skipValidation": False,
    # "devicePrimaryKey":"<<DevicePrimaryKey>>",
    "discoveryUrl": "",
    "IsDebug": True,
}

try:
    with open('/opt/SPARK/iot/secrets.json', encoding="utf-8") as f:
        config = json.load(f)

        UniqueId = config['ids']['uniqueId']
        SId = config['ids']['sid']

        SdkOptions['certificate']['SSLKeyPath'] = config['ssl']['keyPath']
        SdkOptions['certificate']['SSLCertPath'] = config['ssl']['certPath']
        SdkOptions['certificate']['SSLCaPath'] = config['ssl']['caPath']

        SdkOptions['discoveryUrl'] = config['networking']['discoveryUrl']
except Exception as e:
    print(e)
    sys.exit(1)

"""
* sdkOptions is optional. Mandatory for "certificate" X.509 device authentication type
* "certificate" : It indicated to define the path of the certificate file. Mandatory for X.509/SSL device CA signed or self-signed authentication type only.
* 	- SSLKeyPath: your device key
* 	- SSLCertPath: your device certificate
* 	- SSLCaPath : Root CA certificate
* 	- Windows + Linux OS: Use "/" forward slash (Example: Windows: "E:/folder1/folder2/certificate", Linux: "/home/folder1/folder2/certificate")
* "offlineStorage" : Define the configuration related to the offline data storage 
* 	- disabled : false = offline data storing, true = not storing offline data 
* 	- availSpaceInMb : Define the file size of offline data which should be in (MB)
* 	- fileCount : Number of files need to create for offline data
* "devicePrimaryKey" : It is optional parameter. Mandatory for the Symmetric Key Authentication support only. It gets from the IoTConnect UI portal "Device -> Select device -> info(Tab) -> Connection Info -> Device Connection".
    - - "devicePrimaryKey": "<<your Key>>"
* Note: sdkOptions is optional but mandatory for SSL/x509 device authentication type only. Define proper setting or leave it NULL. If you not provide the offline storage it will set the default settings as per defined above. It may harm your device by storing the large data. Once memory get full may chance to stop the execution.
"""


def device_callback(msg):
    """
    * Type    : Callback Function "device_callback()"
    * Usage   : Firmware will receive commands from cloud. You can manage your 
    *           business logic as per received command.
    * Input   :  
    * Output  : Receive device command, firmware command and other device initialize error response 
    """
    global Sdk
    print("\n--- Command Message Received in Firmware ---")
    print(json.dumps(msg))
    cmdType = None
    if msg != None and len(msg.items()) != 0:
        cmdType = msg["ct"] if "ct" in msg else None
    # Other Command
    if cmdType == 0:
        data = msg
        if data != None:
            # print(data)
            if "id" in data:
                if "ack" in data and data["ack"]:
                    Sdk.sendAckCmd(
                        data["ack"], 7, "sucessfull", data["id"]
                    )  # fail=4,executed= 5,sucess=7,6=executedack
            else:
                if "ack" in data and data["ack"]:
                    Sdk.sendAckCmd(
                        data["ack"], 7, "sucessfull"
                    )  # fail=4,executed= 5,sucess=7,6=executedack
    else:
        print("rule command", msg)

def device_firmware_callback(msg):
    global Sdk, device_list
    print("\n--- firmware Command Message Received ---")
    print(json.dumps(msg))
    cmdType = None
    if msg != None and len(msg.items()) != 0:
        cmdType = msg["ct"] if msg["ct"] != None else None

    if cmdType == 1:
        """
        * Type    : Public Method "sendAck()"
        * Usage   : Send firmware command received acknowledgement to cloud
        * - status Type
        *     st = 7; // firmware OTA command Ack status
        *     st = 4; // Failed Ack
        * - Message Type
        *     msgType = 11; // for "0x02" Firmware command
        """
        data = msg
        if data is not None:
            if ("urls" in data) and data["urls"]:
                for url_list in data["urls"]:
                    if "tg" in url_list:
                        for i in device_list:
                            if "tg" in i and (i["tg"] == url_list["tg"]):
                                Sdk.sendOTAAckCmd(
                                    data["ack"], 0, "sucessfull", i["id"]
                                )  # Success=0, Failed = 1, Executed/DownloadingInProgress=2, Executed/DownloadDone=3, Failed/DownloadFailed=4
                    else:
                        Sdk.sendOTAAckCmd(
                            data["ack"], 0, "sucessfull"
                        )  # Success=0, Failed = 1, Executed/DownloadingInProgress=2, Executed/DownloadDone=3, Failed/DownloadFailed=4


def device_connection_callback(msg):
    cmd_type = None
    if msg != None and len(msg.items()) != 0:
        cmd_type = msg["ct"] if msg["ct"] != None else None
    # connection status
    if cmd_type == 116:
        # Device connection status e.g. data["command"] = true(connected) or false(disconnected)
        print(json.dumps(msg))


def twin_update_callback(msg):
    """
    * Type    : Callback Function "twin_update_callback()"
    * Usage   : Manage twin properties as per business logic to update the twin reported property
    * Input   : 
    * Output  : Receive twin Desired and twin Reported properties
    """
    global Sdk
    if msg:
        print("--- Twin Message Received ---")
        print(json.dumps(msg))
        if ("desired" in msg) and ("reported" not in msg):
            for j in msg["desired"]:
                if ("version" not in j) and ("uniqueId" not in j):
                    Sdk.UpdateTwin(j, msg["desired"][j])

def send_back_to_sdk(sdk, dataArray):
    sdk.SendData(dataArray)
    time.sleep(interval)

def device_change_callback(msg):
    print(msg)

def main():
    global SId, SdkOptions, Sdk, ACKdirect, device_list

    try:
        """
        * Type    : Object Initialization "IoTConnectSDK()"
        * Usage   : To Initialize SDK and Device cinnection
        * Input   : cpId, uniqueId, sdkOptions, env as explained above and device_callback and twin_update_callback is callback functions
        * Output  : Callback methods for device command and twin properties
        """
        with IoTConnectSDK(UniqueId, SId, SdkOptions, device_connection_callback) as Sdk:
            try:
                device_list = Sdk.Getdevice()
                Sdk.onDeviceCommand(device_callback)
                Sdk.onTwinChangeCommand(twin_update_callback)
                Sdk.onOTACommand(device_firmware_callback)
                Sdk.onDeviceChangeCommand(device_change_callback)
                Sdk.getTwins()
                device_list = Sdk.Getdevice()

                while True:
                    payload = [
                        {
                            "uniqueId": UniqueId,
                            "time": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%S.000Z"),
                            "data": {
                                "empty": random.randint(2, 8),
                                "taken": random.randint(1, 6),
                                "location": [49, 11]
                            },
                        }
                    ]

                    print(f"Sending payload:{payload}")
                    send_back_to_sdk(Sdk, payload)

            except KeyboardInterrupt:
                print("Keyboard Interrupt, exiting")
                sys.exit(0)

    except Exception as ex:
        print(ex)
        sys.exit(1)

if __name__ == "__main__":
    main()

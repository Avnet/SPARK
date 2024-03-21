# SPARK Requirements & Building

#### Hardware Requirements
- RZ/V2L Evaluation Board Kit
- USB Camera
- USB Mouse
- USB Keyboard
- USB Hub
- HDMI Monitor & Cable
- Matchbox cars
- (Optional) to-scale parking lot. See `SPARK_lot.stl`

>**Note:** All external devices will be attached to RZBoard directly, not requiring driver installation.

#### Software Requirements 
- Ubuntu 20.04 
- OpenCV 4.x
- C++11 or higher

## Application Build, Deploy, Run

>**Note:** Users can skip the build stage by visiting the [release instructions](https://github.com/ljkeller/SPARK/releases).

In order to build, you are expected to have completed [Getting Started Guide](https://renesas-rz.github.io/rzv_ai_sdk/latest/getting_started) provided by Renesas. 

The RZV2L AI SDK docker container is required for building the sample application. By default, Renesas will provide the container named as `rzv2l_ai_sdk_container`.

### Build the software

Run `./build.sh` inside of the RZV2L AI SDK.

### Deploy the software

1) When building from source, you will run ./deploy.sh <rzboard_ip>
2) When using pre-existing releases, follow the [release instructions](https://github.com/ljkeller/SPARK/releases).

### Run the software

1) When running an official release, follow the [operation](https://github.com/ljkeller/SPARK?tab=readme-ov-file#operation) guide
2) When running a personal build, travel to `/home/root/dev/exe` and run the spark executable, optionally running `send_iot_connect.py` if you're configured for IoT connect. Personal builds assume you have the [AI TVM](runtime_deps/libtvm_runtime.so)installed like:

```
├── usr/
│   └── lib64/
│       └── libtvm_runtime.so
```

#### Terminate the Software

- SPARK can be terminated by pressing `Esc` or `q` key on the keyboard connected to the board (while the SPARK ui is showing and in focus)
- Alternatively, User can force close the application using `CTRL+c` in the console (when running SPARK through the console).

## Application: Specifications

#### Model Details

The model used is the custom CNN model. 

```python
        Layer (type)               Output Shape         Param #
================================================================
            Conv2d-1           [-1, 32, 26, 26]             896
         MaxPool2d-2           [-1, 32, 13, 13]               0
            Conv2d-3           [-1, 64, 11, 11]          18,496
         MaxPool2d-4             [-1, 64, 5, 5]               0
            Conv2d-5            [-1, 128, 3, 3]          73,856
           Flatten-6                 [-1, 1152]               0
            Linear-7                    [-1, 2]           2,306
================================================================
Total params: 95,554
Trainable params: 95,554
Non-trainable params: 0
----------------------------------------------------------------
Input size (MB): 0.01
Forward/backward pass size (MB): 0.30
Params size (MB): 0.36
Estimated Total Size (MB): 0.67

```

The network diagram will be as follows: 


<img src=./model_info/model_parking.png width="110" height="480">

###### AI Inference Timing

The AI inference time is 4-7 msec per slot. 

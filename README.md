
# SPARK - Smart Parking AI-Driven RZBoard Kit

Welcome to SPARK, the edge-AI solution for smart EV charging and parking lot management. Bootstrapping your Edge AI with RZBoard and the [RZV AI SDK](https://renesas-rz.github.io/rzv_ai_sdk/2.10/).

![spark_inference_small](https://github.com/ljkeller/SPARK/assets/44109284/8fc8d117-3fa3-469b-b8f1-38e722c21ac1)

## Overview

SPARK is an effective EV charging prototype that integrates a real-time capable convolutional neural network (CNN) for occupancy detection with the RZBoard V2L. The result: a smart parking lot occupancy/EV charging experience for low cost. This system is designed to automate parking lot analysis, streaming edge compute data to your desired target: an information hub like IoT Connect, or an HDMI output. The result: improved user-experience for EV charging customers and parking lot managers.

## Features

- **Real-Time Occupancy Detection**: Utilizing an efficient CNN, SPARK detects vehicle occupancy in real-time, ensuring accurate information on parking availability.
- **User-Friendly Interface**: Easy-to-use interface demo interface: output data to HDMI for simple viewing. Or, integrate with IoT connect for bootstrapped dashboards.
- **Sustainable Solution**: Promotes efficient energy use and supports the growing need for EV infrastructure.
- **Scalable Architecture**: Designed to scale: one image could host up to 200 parking spots. Try adding more parking slots and/or images until your FPS requirement isn't met. Then, scale horizontally with more RZBoards connected to your IoT Connect style HUB!

## Installation

### Standalone Installer
Looking to demo the software? Simply download and install from the [Release](https://github.com/ljkeller/SPARK/releases) instructions.

### Development Installer
**Want to build and modify the demo yourself? You'll have to run the [build script](https://github.com/ljkeller/SPARK/blob/main/build.sh) and/or [deploy script](https://github.com/ljkeller/SPARK/blob/main/deploy.sh) from within the [RZV AI SDK](https://renesas-rz.github.io/rzv_ai_sdk/2.10/)**

```bash
./build.sh && ./deploy.sh [board_ip]
```

## Operation

SPARK operation is as simple as clicking a button and dragging some boxes.

https://github.com/ljkeller/SPARK/assets/44109284/018fd6be-5195-4834-81d6-2894c16b7f5f

## License

This project is licensed under the [MIT License](LICENSE)

## Acknowledgments

- This project wouldn't be possible without OpenCV and [RZV AI SDK](https://renesas-rz.github.io/rzv_ai_sdk/2.10/)!
---

© 2024 Avnet. All Rights Reserved.

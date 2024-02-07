
# SPARK - Smart Parking AI-Driven RZBoard Kit

Welcome to SPARK, the edge-AI solution for smart EV charging and parking lot management. Bootstrapping your Edge AI with RZBoard and the [RZV AI SDK](https://renesas-rz.github.io/rzv_ai_sdk/2.10/).

![SPARK](https://github.com/ljkeller/SPARK/assets/44109284/7083da41-0c51-46cc-8a47-4ca51f296b0a)


## Overview

SPARK is an effective EV charging prototype that integrates a real-time capable convolutional neural network (CNN) for occupancy detection with the RZBoard V2L. The result: a smart parking lot occupancy/EV charging experience for low cost. This system is designed to automate parking lot analysis, streaming edge compute data to your desired target: an information hub like IoT Connect, or an HDMI output. The result: improved user-experience for EV charging customers and parking lot managers.

## Features

- **Real-Time Occupancy Detection**: Utilizing an efficient CNN, SPARK detects vehicle occupancy in real-time, ensuring accurate information on parking availability.
- **User-Friendly Interface**: Easy-to-use interface demo interface: output data to HDMI for simple viewing. Or, integrate with IoT connect for bootstrapped dashboards.
- **Sustainable Solution**: Promotes efficient energy use and supports the growing need for EV infrastructure.
- **Scalable Architecture**: Designed to scale: one image could host up to 200 parking spots. Try adding more parking slots and/or images until your FPS requirement isn't met. Then, scale horizontally with more RZBoards connected to your IoT connect style HUB!

## Installation

Detailed instructions on how to set up and install SPARK on your RZBoard will be provided here. **You are expected to build this from inside the RZBoard AI SDK, or deploy from a Release package**

```bash
# Example installation code
git clone https://github.com/yourusername/SPARK.git
cd SPARK
# further installation instructions
TODO!
```

## Usage

Provide a quick start guide or examples on how to use the system once installed.

```bash
# Example usage code
cd SPARK
TODO
# Add code snippets or command-line examples
```

## License

This project is licensed under the [MIT License](LICENSE) - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

- This project uses several open source libraries:
TODO: ADD
---

© 2024 Lucas Keller. All Rights Reserved.

# Smart Home Simulator

Smart Home Simulator is a software application that simulates a smart home environment. It allows users to create and manage virtual smart devices, such as lights, thermostats, and security cameras, and control them through a user-friendly interface. The simulator provides a realistic experience of how smart home technology works and can be used for testing and educational purposes.

![alt text](https://github.com/YoungMame/smart-home-sim/blob/main/media/functionning.png?raw=true)

## System Architecture
The system arhcitecure is defined in ARCHITECTURE.md, which outlines the modular design of the simulator, including the API layer, core simulation engine, event scheduler, device engine, adapter manager, and supported communication protocols.

## Protocols
The PROTOCOLS.md file describes the communication protocols supported by the simulator, such as MQTT and HTTP, which allow for seamless integration with external applications and devices.

## Technologies Used

### Core
C++: The core simulation engine is implemented in C++ for performance and efficiency.

MQTT Broker: Used for communication between the simulator and external applications or devices.

Fetch library: Used for making HTTP requests to the API.

WebSocket library: Used for real-time communication between the simulator and the dashboard.

### Storage
SQLite: Used for storing device configurations, user data, and simulation state persistently.

### Api
Express.js: The API is built using Express.js, a web application framework for Node.js, to handle HTTP requests and responses.

### Dashboard
React: The dashboard is built using React for a responsive and interactive user interface.



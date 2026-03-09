# Smart Home Simulator

Smart Home Simulator is a software application that simulates a smart home environment. It allows users to create and manage virtual smart devices, such as lights, thermostats, and security cameras, and control them through a user-friendly interface. The simulator provides a realistic experience of how smart home technology works and can be used for testing and educational purposes.

## General Information

![alt text](https://github.com/YoungMame/smart-home-sim/blob/main/functioning.jpg?raw=true)

The system implements a core simulation engine that manages the state of various smart devices and their interactions. Users can add, remove, and configure devices, as well as create automation rules to control device behavior based on specific conditions. The simulator also includes a dashboard for monitoring device status and activity.
It also exposes an API for developpers to develop their own applications on top of the simulator.

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



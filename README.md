# SunSift

SunSift is an energy-aware image sampling and sensing pipeline implemented on microcontrollers. It leverages voltage-based power management and intelligent sample selection to operate effectively under fluctuating energy conditions. The system uses an ArduCAM for image capture, a PMU (Power Management Unit) for threshold-based control, and supports both data transmission and storage strategies.

## Repository Structure

├── PMU/

│   └── ... (content of the PMU directory)

├── Camera.cpp

├── Camera.h

├── TFLModel.cpp

├── TFLModel.h

├── main.ino

├── states.cpp

└── states.h

## Features

- Dynamic energy-aware state transitions (sense, compute, store, transmit, sleep)
- PMU-based threshold monitoring with interrupt pin signaling
- Modular architecture for switching between real-time transmission and local storage

## Hardware Requirements

- Arduino Nano 33 BLE
- ArduCAM Mini 2MP
- External PMU module with 3 GPIO threshold outputs

## License

This project is licensed under the [MIT License](LICENSE).

---

**Demo Video**: [Watch on Google Drive](https://drive.google.com/file/d/1CBzY8Ofg8hZi3UUBs5W5QU0BYAqKNcIU/view?usp=sharing)  

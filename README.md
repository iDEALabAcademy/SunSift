# SunSift

SunSift is an energy-aware image sampling and sensing pipeline implemented on microcontrollers. It leverages voltage-based power management and intelligent sample selection to operate effectively under fluctuating energy conditions. The system uses an ArduCAM for image capture, a PMU (Power Management Unit) for threshold-based control, and supports both data transmission and storage strategies.

## Repository Structure

All files related to the Power Management Unit (PMU) are located in their own dedicated PMU/ folder. This isolates the power management logic from the other core application files, such as main.ino, Camera.cpp, and TFLModel.cpp, which reside together in the main directory. Based on your board use the related main file. PMU_sus contains the code that can be used with the sustaiable setup. Receiver code is meant to run on central device (like your computer) which receives and saves the captured image. See received_image for a sample.

## Features

- Dynamic energy-aware state transitions (sense, compute, store, transmit, sleep, ...)
- PMU-based threshold monitoring with interrupt pin signaling
- Modular architecture for switching between real-time transmission and local storage

## Major Hardware Requirements

- Arduino Nano 33 BLE or Nano ESP32
- ArduCAM Mini 2MP
- Everspin SPI MRAM

## License

This project is licensed under the [MIT License](LICENSE).

---

**Demo Video**: [Watch on Google Drive](https://drive.google.com/file/d/1CBzY8Ofg8hZi3UUBs5W5QU0BYAqKNcIU/view?usp=sharing)  

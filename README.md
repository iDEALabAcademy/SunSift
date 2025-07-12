# SunSift

SunSift is an energy-aware image sampling and sensing pipeline implemented on microcontrollers. It leverages voltage-based power management and intelligent sample selection to operate effectively under fluctuating energy conditions. The system uses an ArduCAM for image capture, a PMU (Power Management Unit) for threshold-based control, and supports both data transmission and storage strategies.

## Repository Structure


- **main_sense_transmit.ino**: Captures and transmits image data under favorable power conditions.
- **main_sense_store.ino**: Captures and stores image data locally for future transmission or processing.
- **PMU/**: Contains a separate sketch (`PMU.ino`) responsible for managing voltage thresholds and generating interrupts to guide the system’s behavior.

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

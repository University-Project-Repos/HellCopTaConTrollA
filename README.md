# ENCE361 Helicopter Rig Controller

A real-time kernel and interrupt-driven embedded-C program for remotely accessing and
controlling the state, orientation and operation of a model helicopter using 

* PWM motor control
* ADC sensor inputs
* quadrature decoding for rotor position tracking
* round-robin task management scheduling
* state machine logic
* multi-channel PID for stability control
* UART communication
* OLED display for real-time data
* timer-based operations
* de-bouncing for reliable input data
* infrared distance sensor for obstacle detection

A model helicopter can be controlled to perform varying operations and aerial manoeuvres, such as
- ignition, idling, vertical lift-off, hovering, landing and powering down
- ascending and descending to any altitude within a specified ground-ceiling range
- yaw rotating 360° to any referenced point in either clockwise or counter-clockwise direction

### Authors

* [Augustus Ellerm](https://github.com/GusEllerm)
* [Andrew Limmer-Wood](https://github.com/Schrekse)
* [Adam Ross](https://github.com/r055a)

# Instructions

## Requirements

* [_Tiva C Series TM4C123G LaunchPad_](https://www.ti.com/tool/EK-TM4C123GXL) 32-bit microcontroller with an ARM Cortex-M4 CPU
* [_Orbit BoosterPack_](https://digilent.com/reference/orbit_boosterpack/orbit_boosterpack)
* Numerous excluded files for installing to a launchpad, such as OrbitOLED for the display.
* helicopter, etc.

## Install

Compile using Code Composer Studio (CSS) IDE with Tivaware.

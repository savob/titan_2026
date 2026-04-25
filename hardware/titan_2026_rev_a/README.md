# TITAN Board

The main custom board for TITAN is designed to be a HAT (**HA**rdware on **T**op) for the Raspberry Pi 3s we use for video feeds. This baseform form factor was inherited from the 2022 version. At the heart of the HAT, there is an STM32 microcontroller which communicates with the RPis and various sensors around TITAN via the various connections present.

## Functionality

These HATs perform four main functions:

1. Regulate power for their RPi and monitor from the battery
2. Facilitate communication between RPis via an STM32 microcontroller
3. Collect data from TITAN using the STM32 microcontroller
4. Handle telemetry broadcast out of TITAN

**Functions 2 to 4 only need to be performed on the "primary" (front rider's) unit.** The secondary (rear rider's) only needs to merely power the RPi and display, and connect to the primary board.

### Power Regulation

These boards receive power directly from LiFePO4 batteries which needs to be regulated down to 5&nbsp;V for the RPi to use, and passed to the display.

The main 3.3&nbsp;V rail for logic will be taken from the RPi for board logic. However there will be an linear regulator and power switch to allow for the USB to power the board safely in the absence of an RPi.

Historically we used 3S packs with a nominal voltage of 9.9&nbsp;V which when really discharged would lead to some screens dimming their backlights, although never fully turning off. So for 2026 I am considering going up to 4S to prevent this happening.

### Sensors

The board hosts several sensors to monitor different things around TITAN. Some sensors are located off board to be better positioned to capture their data such as the wheel sensors. A summary of the sensors interfaced directly with the STM32 in the system is:

- Encoder based wheel speed and distance traveled
- GPS based speed/distance
- Battery levels
- CO2 levels
- Brake temperatures
- Ambient temperature and humidity

### Communication

The STM32 acts a server essentially for data, serving and accepting data from the different system endpoints. Communication with the two RPis is done via UART, and out of TITAN using nRF24 radios.

### Configuration

All boards will be assembled identically, so setting a board for operation in a "primary" or "secondary" role will be needed.

This is partly achieved through hardware design with the `ROLE` signal, but some solder jumpers are needed to portion off the I2C bus on the secondary board to avoid address collisions when connected.

Depending on the displays used there is a solder jumper to provide them with power from either the 5&nbsp;V rail or directly from the battery.

### Standalone Operation

Although not an critical design constraint, I would like these boards to be able to operate on their own for testing and even use as downlink boards for the crew outside TITAN. So I have incorporated a USB-C connection and some power switches to safely connect to the USB under any configuration of the board.

## Changes from 2022

There are a multitude changes between the 2022 and 2026 versions of the TITAN board, however my desire is to make them all transparent to the system at large (RPis), and minimize the complexity of firmware rewrites for the STM32. In approximate order of implementation:

- Added isolators to be used for inter board UART lines
- Integrated a GPS module
- Integrated environmental sensors
- Using proper wire to board connectors for everything rather than expecting to soldering some wires to the PCB
- Added sensors to monitor power use of different parts of the system
- Changed the main microcontroller to an STM32 with more GPIO (STM32F105R)
- Removed reverse polarity input protection for simplicity
- Removed input undervoltage protection, to accommodate varying batteries
- Added on board regulator and power switch ICs to run off USB power if not mounted on an RPi ("standalone")
- Board "role" is determined in hardware based on the connection between the primary and secondary
- Added jumper to select power output to the display connector

These cover all the changes I felt were really needed on TITAN. There was only one change that I would like but couldn't justify: integrating the nRF24L01+LNA radio used for telemetry. It would needlessly take me more time to draw the schematic, spec the parts, and then layout while not meaningfully improving any performance metric while greatly increasing the cost compared to just getting a handful of the preassembled modules online. _I'll put aside the vain desire for a majestic monolithic PCB, for now._

# TITAN Board

The main custom board for TITAN is designed to be a HAT (**HA**rdware on **T**op) for the Raspberry Pi 3s we use for video feeds. This baseform form factor was inherited from the 2022 version. At the heart of the HAT, there is an STM32F103C microcontroller which communicates with the RPis and various sensors around TITAN via the various connections present.

These HATs perform four main functions:

1. Regulate power for their RPi and monitor from the battery
2. Facilitate communication between RPis via an STM32 microcontroller
3. Collect data from TITAN using the STM32 microcontroller
4. Handle telemetry broadcast out of TITAN

**Functions 2 to 4 only need to be performed on the "main" (front rider's) unit.** The secondary (rear rider's) will be configured to merely power the RPi and display, but the microcontroller will be bypassed to connect to the primary.

## Power Regulation

These boards receive power directly from LiFePO4 batteries, historically we have used 3S packs with a nominal voltage of 9.9&nbsp;V, this needs to be regulated down to 5&nbsp;V for the RPi to use, and boosted to 12&nbsp;V for the displays - both regulated on board with switch-mode supplies.

The main 3.3&nbsp;V rail will be taken from the RPi for board logic. However there will be an linear regulator and power switch to allow for the USB to power the board safly in the absence of an RPi.

_Unlike the 2022 design, there will likely be no reverse polarity protection as the battery connector should be good enough. Undervoltage protection may also be removed._

## Sensors

The board hosts several sensors to monitor different things around TITAN. Most sensors are loacted off board to be better positioned to capture their data such as the wheel sensors. A summary of the sensors interfaced directly with the STM32 in the system is:

- Encoder based wheel speed and distance travelled
- GPS based speed/distance
- Battery levels
- CO2 levels
- Brake temperatures
- Ambient temperature and humidity

In this version I am looking to try and integrate more of the sensors onto the board such as GPS, and add new ones like power monitors for boards.

## Communication

The STM32 acts a server essentially for data, serving and accepting data from the different system endpoints. Communication with the two RPis is done via UART, and out of TITAN using nRF24 radios.

An improvement from the previous version of TITAN is that isolators will be used for each UART connection to prevent damage from different power states across ats of the system.

Also in the previous version of TITAN the nRF24 radio was soldered as an off the shelf module, for this board they're being considered for proper integration.

## Configuration

All board will be assembled identically, so setting a board for operation in a "primary" or "secondary" role will likely be done using solder bridges on designated jumpers.

## Standalone Operation

Although not an critical design constraint, I would like these boards to be able to operate on their own for testing and even use as downlink boards for the crew outside TITAN. So I have incorporated a USB-C connection and some power switches to safely connect to the USB under any configuration of the board.

# Arduino Nano Downlink

A simple serial to nRF24 bridge for receiving TITAN braodcasts onto a host PC. Potentially even sending data back to TITAN using acknoledgement packets.

This firmware exists primarily to aid in the development of the radio broadcasts of TITAN by using a known good library. The target hardware is the super old telemetry boards I made to have an nRF24 module backpack on an Arduino Nano, which used the default pinout for the library at the center of this, [RF24](https://github.com/nRF24/RF24).

_The baseline code was taken from the firmware unit tests for TITAN 2022._

## Hardware preparation

This project is prepared for an Arduino Nano with the original bootloader, although it can be easily reconfigured to work for a Nano with the new bootloader or even the classic Uno using the same pinout.

The connections needed to be made are as follow. They are listed in order from pin 1 on the nRF24 (indicated with the square pad) to its neighbour (pin 2) before advancing to the next row.

| Arduino | nRF24 |
| ---: | :--- |
| GND | GND |
| 3.3V | VCC |
| 7 | CE |
| 8 | CSN |
| 13 | SCK |
| 11 | MOSI |
| 12 | MISO |
| 2 | IRQ |

>[!NOTE]
> On the Arduino Nano pin 13 (SCK) is used for the built in LED so it cannot be used in this code, and will flash when there is any SPI activity between the Arduino and radio.

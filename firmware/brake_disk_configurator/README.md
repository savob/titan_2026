# Brake Disk Sensor Configuration Utility

This firmware was made to allow TITAN's brake disk sensors to be configured and tested prior to operation. It is operated using the UART connection via the debug connector, since you'll need that to flash the MCU anyways. Using the debugger will also safely preserve your connection to the system as you potentially power cycle TITAN.

>[!CAUTION]
> Always make connections when the system is unpowered! Please avoid "hot-plugging" the sensors into a powered TITAN board, this may cause damage to the electrical hardware.

## How to Use the Configuration Software

Once you have the firmware flashed to a TITAN 2026 Rev. A board and with the debugger still connected, open a serial terminal on your computer and connect to the debugger's serial port at a **Baudrate of 115200**. Then enter commands as desired, _no need for any newlines or such_. The board processes commands once every couple of seconds so please be patient! All commands follow format `CAA,P` where:

- `C` is the command character
- `AA` is the target senor's I2C address in hexadecimal, e.g. `5A` instead of `90`, _if appicable_
- `P` is an optional parameter _(if not needed for a command the leading comma may be ommited)_

>[!TIP]
> Occaionally only the command character is captured from the user's input, if this happens simply repeat the message until properly received - no harm will come from this. There is no user input that can cause damage to the system.

| Command character | Operation | Address Needed | Parameter |
| :---: | --- | --- | --- |
| `H` | Print help message _(a basic summary of this table)_ | No | None |
| `S` | Scan the bus and report any responsive addresses | No | None |
| `O` | Read object temperature from the sensor | Yes | None |
| `A` | Read ambient (sensor's own) temperature from the sensor | Yes | None |
| `E` | Set object's emissivity as P | Yes | Decimal digits of emissivity (as many digits as desired), e.g. `E5A,769` to set emmisivity of 0.769 at `0x5A`, use 0 to set 1.000 |
| `C` | Change sensor address | Yes | New address in hexadecimal |

>[!NOTE]
> Changes to address and emmissivity properly take effect after the sensor is power cycled.

## General Procedure

If I were doing this I would follow this procedure roughly, the important thing is to ensure that we have sensors at the right addresses above all else so they don't have address collisions inside TITAN since all **sensors are shipped with a default address of 0x5A**. Emmisivity and temperature tests can follow later.

1. With TITAN powered off connect the front brake disk temperature sensor to the bus using the right connector
2. Power up TITAN and run a scan using the appropriate command, only one address should be listed, likely `0x5A`
3. Change this address to the desired address (likely `0x01`, check with a firmware nerd) for the front sensor using the appropriate command
4. Power cycle the system
5. Perform a scan to confirm the address change took effect
6. Power down the board and disconnect the front sensor
7. Repeat steps 1 through 6 with the rear wheel sensor instead, to set it to an address of `0x02` (double check with the firmware nerd)
8. With TITAN powered down connect both sensor
9. Power up the system and conduct a scan and check that both sensors are reporting at their appropriate addresses
10. _(Optional, but recommended)_ Adjust the emissivity of both sensors to match the brake disk material, in 2022 we used `0.600`.

With this done, the rest is really just playing around checking it is reading appropriate temperatures when objects are infront of a given sensor.

>[!IMPORTANT]
> In summary the following parameters should be set before TITAN goes out for a run.
>
> - Front wheel address should be `0x01`
> - Rear wheel address should be `0x02`
> - Both wheels should have an emissivity of `0.600`

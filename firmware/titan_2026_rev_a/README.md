# TITAN 2026 Code

THis is the code meant to operate the entire system. It is meant to be flashed onto bothe boards present in the bike, regardless of position, they will determine their role based on the connections made to them on boot and act appropriately for their role.

## Primary Role (Front Rider)

If determined to be on the primary video system board, the microcontroller will assume the task of operating all real-time tasks for TITAN. Collecting all the data from around the bike's various sensors, processing the data into usable formats, handling communication between the microcomputers running the video systems, and even broadcasting data over radio.

When starting up it will check for all the sensors it expects in the system, should any fail to be identified or configure themselves the microcontroller will ignore them and continue operation. Recovery after boot is not a behaviour yet.

## Secondary Role (Rear Rider)

When the microcontroller determines it is serving on the secondary system is simply illuminates the status LEDs to indicate this and enters an infinite loop of idling so it doesn't interfere with the operation of the system.

## Status LEDs

There are three status LEDs located centrally on the board used for various purposes. They are labelled `STATx`, `STAT1` is red while the other two are white. All these LEDs can be dimmed.

>[!TIP]
>The same status LED state may indicate different things depending on the microcontroller's role so be aware of what role a board will have before trying to decipher their status LED meaning.

### `STAT1` (Red)

This is generally reserved to indicate an issue has occured.

- If `STAT1` is flashing quickly (5&nbsp;Hz) while other LEDs are off, then something has gone critically wrong and the microcontroller will reboot. **This may result in the loss of some data accumulated on the microcontroller itself**, namely wheel distance.
  - _This state has not been observed in normal operation, only during boot related to the GPS sending malformed data._

### `STAT2` and `STAT3` (White)

General purpose indicators, used for just about anything that isn't an issue.

- If only `STAT3` is illuminated then the microcontroller has identified it is serving as the secondary microcontroller and is essentially inactive
- If both `STAT2` and `STAT3` are illuminated _(without `STAT1`)_ then the microcontroller is operating as a primary microncontroller

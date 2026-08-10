# TITAN 2026 Code

THis is the code meant to operate the entire system. It is meant to be flashed onto bothe boards present in the bike, regardless of position, they will determine their role based on the connections made to them on boot and act appropriately for their role.

## Primary Role (Front Rider)

If determined to be on the primary video system board, the microcontroller will assume the task of operating all real-time tasks for TITAN. Collecting all the data from around the bike's various sensors, processing the data into usable formats, handling communication between the microcomputers running the video systems, and even broadcasting data over radio.

When starting up it will check for all the sensors it expects in the system, should any fail to be identified or configure themselves the microcontroller will ignore them and continue operation. Recovery after boot is not a behaviour yet, so if a sensor is reconnected during operation the system will not notice until rebooted.

>[!IMPORTANT]
> The primary board uses a watchdog timer to ensure it does not get frozen and as a recovery mechanism after a critical error. This means that if the microcontroller freezes or otherwise fails to reset the watchdog timer before it hits zero, the microcontroller will reboot and try to recover with a clean(er) slate. The system has been configured that if such a watchdog reboot occurs then the vehicle data will be preserved into the next boot preventing the loss of distance covered and other measurements. This reboot will not interrupt the video systems in any meaningful way other than a temporary break in data updates.

## Secondary Role (Rear Rider)

When the microcontroller determines it is serving on the secondary system is simply illuminates the status LEDs to indicate this and enters an infinite loop of idling so it doesn't interfere with the operation of the system. No information is collected by or passed through the microcontroller in this role.

## Status LEDs

There are three status LEDs located centrally on the board used for various purposes. They are labelled `STATx`, `STAT1` is red while `STAT2` and `STAT3` are white. All these LEDs can be dimmed.

>[!TIP]
>The same status LED state may indicate different things depending on the microcontroller's role so be aware of what role a board will have before trying to decipher their status LED meaning.

Below is a table summarizing various status LED states. If "X" is used then that light doesn't matter to communicate something - for example with non-critical errors.

| `STAT1` (Red) | `STAT2` (White) | `STAT3` (White) | Meaning |
| :---: | :---: | :---: | :--- |
| Flashing at 5 Hz | OFF | OFF | Critical error encountered, **microcontroller will reboot momentarily!** _Note: this will not interrupt the video systems._ |
| Cycling dimmly | X | X | Non-critical issue found, sensor system will continue to fulfil its role in a dimished state. _A sensor might be disconnected._ |
| OFF | X | X | No issues observed in by this board as this role. |
| X | OFF | ON | Board detected role as secondary board. |
| X | Cycling | ON | Board is operating as primary system, **GPS lock in progress.** |
| X | ON | ON | Board is operating as primary system, **GPS lock completed.** |

## Potential Improvements

The code as is, can function as a drop in replacement for the 2022 boards (with modified connectors). There are a few minor features or improvemetns that are being considered for future firmware versions:

- [ ] Have the secondary board firmware check if it can find the INA219 IC on the bus to alert users to inproper configuration somehow
- [ ] Make use of the USB connection perhaps for additional debugging interface
- [ ] Autodetect the SCD-4x variant used on the board, and add altitude adjustment

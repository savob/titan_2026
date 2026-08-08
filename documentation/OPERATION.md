# Operating TITAN

TITAN was designed and tested to be a pretty simple system to operate by requiring no user input beyond a the input to power on, once assembled correctly. This has been preserved with this update to the system, which aimed to not disrupt any of the established TITAN protocols.

>[!NOTE]
> This document assumes the TITAN 2026 electrical system has already assembled and setup. For instructions on how to assemble and setup the TITAN hardware please refer to the [assembly document](./ASSEMBLY.md).

## Batteries and Power

_The system's life force!_ The batteries TITAN was **designed to use 3S LiFePO4 batteries**, so I recommend they continue to be used. Their working range of voltages are 9.0&nbsp;V to 10.8&nbsp;V with a nominal voltage of 9.9&nbsp;V. All batteries should be charged fully prior to any use of TITAN.

Although designed for 3S LiFePO4 batteries, the input of TITAN 2026 can handle a wide range of voltages and still operate without issue. Despite the labeling of `12V MAX`, the RPi board can tolerate between 5&nbsp;V and 17&nbsp;V on the input and provide the needed power for all systems. The stated 12&nbsp;V limit is imposed by the monitor which depending on how the board is configured may draw power from the regulated 5&nbsp;V rail _(in which case the board's own limits still apply)_ or connected directly to the battery which would mean the battery must satisfy the monitor's power requirements. In the the 2022 system the monitors were all designed to handle 12&nbsp;V at most so I kept that warning on the board should the same monitors continue to be used, **please check the monitors used for their power requirements - twice!**

>[!TIP]
> LiFePO4 chemistry batteries are preferred for TITAN due to their safer nature. Their chemistry has a larger range of environmental conditions where it is stable and less dramatic failure modes should the worst come to happen which affords our riders more time to vacate the vehicle in an emergency.

The exact power draw of TITAN is approximately 10&nbsp;W per RPi vision system, about half that for the analog back up system. As such **a 1000&nbsp;mAh 3S LiFePO4 battery can supply a single vision system an hour of operation**. This is plenty for most of our 20 minute heats - but warrants changes for longer testing sessions.

There is circuitry to allow TITAN RPi HATs to seamlessly and safely switch between battery and USB power when in use, should both sources be connected to the board then the battery is selected for power. It is not recommended to use USB power to power an RPi through the HAT as the USB-C connector is configured to only request around 5&nbsp;W of power - enough to run the sensor system.

## Start Up

With TITAN assembled correctly the process to launch the system is pretty straight forward.

> [!CAUTION]
> Failure to follow the battery connection sequence properly may damage the hardware.

1. Ensure batteries are fully charged
2. Plug in the batteries following this exact order. Each system should automatically begin booting when their battery is connected
   1. Primary (front) system
   2. Secondary (rear) system
   3. Backup system
3. Press the power button (`PWR_RPi`) on RPi systems to boot them if needed - for example if they were shutdown without battery removal

The backup system should show the video feed immediately when turned on. The RPi systems will take about 30 seconds to put the video on screen, which will be preceded with boot messages and a printing of the system's configuration.

## Shut Down

Whenever power is going to be removed from TITAN the following procedure should be followed. Be it at the end of a run, or to conserve battery when staging before a run.

> [!CAUTION]
> Failure to follow this properly may damage the hardware or risk corrupting the data on the RPis.

1. Press the power button (`PWR_RPi`) on the HAT
2. Wait for the video feed to go off and the RPi to shut down properly
   - The video feed should disappear within seconds after the button is pressed, if not press the button again
   - Repeated presses of the off button will not adversely affect the system
   - If the system doesn't shutdown within a minute you may need to connect a keyboard or simply pull the power risking data loss
   - As the RPi shuts down its status LED will flash ten times at 2&nbsp;Hz before going out, that signals it is safe to remove power
3. If desired, disconnect the batteries. They **MUST** be removed in the reverse order they were installed in:
   1. Backup display
   2. Secondary system
   3. Primary system

If the batteries are left in place after the RPis are shutdown, the RPis can be turned on at the press of the power button (`PWR_RPi`). This is useful to do for staging before a run. There is no required order to follow when turning on RPis in this manner because the power supplies have all stabilized.

## Transferring Data To or From the RPis

My _(Savo's)_ preferred method to transfer data was to remove the RPi's micro SD card _(after a proper shutdown with battery removal!)_ and insert it into my computer and directly copy the data of interest onto the computer that way. The only issue is that you'll likely need a Linux machine to recognize the file system the RPis use. Having the SD card in my computer also made it easy to edit some of the scripts to handle things like new power pedals.

Other options include network access or using the RPis as a normal desktop and a USB drive to ferry the data to another computer.

>[!IMPORTANT]
> To get to the desktop of a TITAN RPi one can press the RPi's power button (`PWR_RPi`) during operation of the vision system to stop the vision system.

If the button is not working, one can try to manually enter the following sequence of command blindly into the terminal with a keyboard to shut the vision system down. _Best of luck console cowboy!_

```bash
sudo pkill bike
sudo pkill python
```

>[!TIP]
> Entering `startx` in the terminal will start the graphical interface if desired for easier operation of the RPis. _Brace yourself for a slow computer experience._

### Log Files and Locations

By default the TITAN system records data in two locations, one for the videos and another for the ride data.

> [!NOTE]
> All recorded files are timestamped in their names with the start of the run. However the **RPis do not keep time when they are off, so these times will likely lag behind the true start time**. To minimize this time discrepancy the RPis should be allowed to connect to the internet to synchronize their clocks regularly.

| Data | Location | Format |
| --- | --- | --- |
| Ride data | `/home/pi/Desktop/vision/bikelogs/` | `.csv` |
| Video recordings | `/home/pi/Videos` | `.h264` |

### Video Conversion

To convert from `.h264` recordings to the more commonly accepted `.mp4` one may use the following `ffmpeg` command with the appropriate file names substituted for "video":

```bash
ffmpeg -r 60 -i video.h264 -c:v copy video.mp4
```

>[!NOTE]
> The video recordings are known to have portions where the video seems to skip ahead for split seconds. This is likely due to the recording program getting overwhelmed or some hiccup in the `.mp4` conversion process.
>
> Furthermore, the video and overlay/logging are run as separate process so the videos will lack the overlay the rider's had. Also their starting time stamps will likely differ and need a little effort to align if one tries to reconstruct the overlays later.

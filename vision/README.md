# Vision Code

This folder is the collection of vision system code running on the RPis. Split between C and Python code.

>[!NOTE]
> Although these files can be viewed and edited off RPis they will only really compile and work properly on RPi 3 microcomputers due to the dependence on some hardware specific libraries, especially for the overlay functionality.

## Setup

>[!IMPORTANT]
> There is a bit of setup needed for this code to work assuming the system is not derived from an image of TITAN in 2022. However I am hazy on the details so this is my best guess.

To setup the ANT receiving system please follow the instruction in [HPVDT's fork](https://github.com/hpvdt/openant) of another fork of OpenAnt.

To configure the C build system for the overlay system one will likely need to execute the following shell commands, although I am unfortunately not certain if they're what I did all those years ago.

```bash
sudo apt install cmake
sudo apt-get install libraspberrypi0
sudo ldconfig -n /opt/vc/lib
```

## Operation

The RPis are configured to boot this system automatically when powered by executing the `titan_startup.py` script when booted (done by editing [rc.local](https://linuxhint.com/use-etc-rc-local-boot/)). To speed this process up, the RPis are configured to boot to command line (CLI) rather than starting the graphical user interface; we believe this also slightly improves the speed of the system.

`titan_startup.py` then launches a series of other processes that collectively represent the TITAN vision system.

- `power_button.py` watches for presses of the 'PWR' button
  - If tapped briefly it merely stops the vision system (useful for development). Entering `startx` into terminal is then needed to start the desktop environment.
  - If held for a few seconds it will shut down the RPi. Presing the button again will start the RPi.
- `camera.py` starts the camera feed and camera recordings

Once these are running, the startup script then responsible for determining whether the RPi is supposed to be the front or rear rider's system based on the presence or absence (respectively) of an ANT+ receiver, and launching the overlay program, `bike.bin`, accordingly by passing it the right arguments.

- If it is the rear system (no ANT+ modules connected), then the startup script just launches `bike.bin` with the arguments needed for the rear system.
- If it is the front system (ANT+ module present), then the startup script launches the `titanant.py` script to read the ANT+ data and pipe it into `bike.bin` to be overlaid and fed to the STM32.

**NOTE: All these scripts/executables are run as separate processes so that if one fails, the others should continue unhindered. Namely the video is never interrupted to the riders.**

## Overlay Code

The overlay code is the main product of this folder, and written in C. It is split up across several function specific C files. Handles communication with the STM32 for data, overlay drawing, and logging as requested.

- `main` - Main file. Reads in arguments provided to determine behaviour (e.g. 'front' or 'rear' system, record logs or not) and then run the main loop to create overlays.
- `antInterface` - Used to process the piped in ANT data from `titanant.py` for use on the overlays and passing on the the STM32
- `overlay` - Responsible for the actually drawing of the overlay. Uses the chipset driver library unique to the RPi 3 hardware to draw the overlay over the video feed. This limits its use to only the 3B or 3B+, and `bike.bin` must be compiled specifically for each model.
- `logging` - Used to write a CSV file of all data collected on the bike
- `racesim` - A C version of our Race Simulation file used to determine how our bike should perform and then compare to how it actually is to see if we're exceeding expectations or not. **UNTESTED**.
- `serialComs` - Responsible for handling communication with the STM32
- `timeTrial` - Used to monitor timing of the code to see how long different sections take to execute. *Used for development purposes, not needed for the riders.*

The code can be edited on the RPi itself, and this is fine for any small tweaks such as some hardcoded constants like wheel radius. However for any larger changes it will likely be easier to offload the editing onto a more capable computer to then copy the changed source code onto an RPi. The exact method to do this is up to you, but I generally opted to move the SD card between the RPi and my laptop.

### Compiling the Overlay Code

With any changes to the overlay source code, one will need to compile a new `bike.bin` for them to take effect.

>[!IMPORTANT]
> Regardless of how the code is edited, it **MUST be compiled on an RPi 3 itself**. This is because overlay code makes use of low level video drivers (VideoCore) for the Broadcom chip that runs the RPi 3 computers, as well as some specific features for some timing code. It is for this reason that many of the `#include` statements and constants will likely be flagged as errors on a non-RPi computer.

The compilation process is based on using a `Makefile` so to generate a new `bike.bin` simply execute the following command in this folder.

```bash
make
```

The full procedure for this is roughly:

- Replace the contents of the `/home/pi/Desktop/vision` on the RPi SD card with the new file(s)
- Reseat the SD card in the RPi and boot it
- If the video system starts, tap the power button on the RPi HAT to kill the video system and reveal the terminal
- Navigate to the vision folder with `cd /home/pi/Desktop/vision`
- Run `make` in that folder to compile the new binary
- Reboot the system if no errors occur

# Raspberry Pi Configuration

The hardware we use for TITAN are Raspberry Pi Model 3 computers, any subvariant is acceptable (A, B, B+). The system image we have been using for years is for 16&nbsp;GiB microSD cards.

>[!WARNING]
> Much of what was used is now depreciated or obsolete so be careful when updating the images or downloading libraries.

The version of Raspbian _(yes, Raspbian - not Raspberry Pi OS)_ used is "Full" 64-bit version of 11.4 - "Bullseye". Likely the 2022-04-07 release.

The Raspberry Pis were configured years ago with no proper record other than the console log to work with - copied here as `TITAN.bash_history`. From it I've gathered the following pointers on how it was configured.

Check if the VideoCore library and examples are present in the system at `/opt/vc/`. If not try the following remedies:

- Run this in terminal `sudo apt-get install libraspberrypi0`
- Download the `userland` repository at this commit, the most recent before the 2022 competition, although likely already on the RPi. https://github.com/raspberrypi/userland/tree/54fd97ae4066a10b6b02089bc769ceed328737e0

With that sorted run the following commands:

```bash
sudo apt install cmake
sudo ldconfig -n /opt/vc/lib
```

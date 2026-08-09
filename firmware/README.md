# Microcontroller Code

This folder contains the firmware projects for the embedded system within TITAN going into competition for 2026, targeting the STM32 microcontroller on board. The base for these is the original project from 2022, which was written using the Arduino IDE within the PlatformIO system. Unfortunately I didn't check it before upgrading the microcontroller but the STM32F105 series **lacks any compatibility with the Arduino framework** so a direct port is impossible at present.

This has forced my hand into remaking the project using STM32CubeIDE which is now complete. All code going into the embedded system will be written in C with potentially some C++.

>[!IMPORTANT]
> These firmware projects will generally have "Debug" and "Release" build/upload options. Whenever programming a board to be used in the field **ALWAYS select the "Release" version** as they will be more optimized by the compiler and have some debugging systems (e.g. testing `printf()` lines) stripped out in the pursuit of performance. Only use "Debug" when actively working on a development or performance is not important.

| Project | Purpose |
| --- | --- |
| `titan_2026_rev_a` | Principle firmware to run TITAN boards during races (for both primary and secondary boards) |

## Programming the Boards

To program the STM32 based boards, some version of an STLink is needed to form the connection via a ribbon cable. The [STLINK-V3MINIE](https://www.st.com/en/development-tools/stlink-v3minie.html) is a good tool for this and can be had for a reasonable price. There should be a few of these around the workshop.

>[!TIP]
> The MINIE will not supply power so connect a battery or USB cable to targetted systems. When connecting the ribbon cable to the RPi HAT boards (Rev. A) then the cable is connected such that it covers the LEDs.

If the MINIE is not available there are other more expensive variants or even STM32 development boards also have these built in and can be easily repurposed to program other boards.

>[!NOTE]
> In the past we used bootleg STLink V2 programmers but these will not be recognized and used by STM's software. These also lack consistent pinouts and the protections offered in official offerings.

With the programmer connected to the target board you will then be able to upload firmware using the [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (compiling from source code) or the [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (if you have precompiled binaries). For more details on the uploading process refer to ST's own documentation depending on the software used.

## Software Used

All firmware was written using the [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html), the projects were configured using [STM32CubeMX](https://www.st.com/en/development-tools/stm32-configurators-and-code-generators.html). These, and most of ST's software is all crossplatform and free to use.

Of these two, STM32CubeIDE is needed to tweak, compile, and then upload firmware. MX is less likely to be needed since that is only used to initally configure the projects and generate code for their hardware, e.g. setting pin functions or configuring communication buses. These configurations don't change significantly except with hardware revisions.

As mentioned earlier the [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) can be used to flash microcontrollers and perform some basic debugging. The problem is that is can only really upload pre-compiled firmware files so you cannot change the firmware using it, for example to adjust wheel diameter.

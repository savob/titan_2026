# Microcontroller Code

This folder contains the firmware projects for the embedded system within TITAN going into competition for 2026, targeting the STM32 microcontroller on board. The base for these is the original project from 2022, which was written using the Arduino IDE within the PlatformIO system. Unfortunately I didn't check it before upgrading the microcontroller but the STM32F105 series **lacks any compatibility with the Arduino framework** so a direct port is impossible at present.

This has forced my hand into remaking the project using STM32CubeIDE, and I may consider an RTOS framework since I'll anyways be starting from scratch.

All code going into the embedded system was written in C/C++.

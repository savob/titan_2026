# TITAN STM32 Code

This is the code for running the main microcontroller in TITAN, charged with not only collecting data but also passing it around the system like a router through a call and response system to the RPis inside TITAN or via radio to the crew.

## Data Collection

The STM32 served as the primary data collector in the system, critically for the real-time data like wheel speed as well as interfacing with the array of sensors and radios employed. Data streams monitored by the STM32:

- Optical encoder for wheel
  - Speed
  - Distance traveled
- Temperature and humidity of external air using a DHT22 module
- Battery levels
- GPS data using MTK3339 module
  - Position coordinates
  - Speed (as a backup to encoder)
- Brake disk temperatures using MLX90614 sensors
- Internal CO2 levels from an MH-Z19C sensor

In addition to this data collected directly by itself, it also acts as a messenger to pass ANT+ data between the RPis. For each of our two riders the following data is provided to the STM32: 

- Heart rate (BPM)
- Instantaneous power output (W)
- Cadence (RPM)

## Communication Format

To minimize unneeded traffic and offer flexibility for the systems to cater the data streams to suit there needs TITAN was designed to use a call and response system: connected devices would send a query code e.g. `s` for current speed, and then the STM32 would respond with the appropriate data in human readable form with a leading character to indicate the length of the message, i.e. `$123.3`. Data could also be sent to microcontroller by using a posting code (usually the capital letter for a field) and the payload, so `S$101.0` would have the microcontroller record the speed as 101.0 km/h, useful for posting data the STM32 wouldn't be able to collect on it's own like biometric data to then share with the rest of the system.

This worked great at first for development as one could easily test the system with a UART bridge. However it was cumbersome and slow for actual operation as each field needed to be queried independently and the STM32 had to spend significant compute parsing and creating human readable representations, so we've move to more condensed, binary encoded data exchanges to speed up communication between the RPis, telemetry, and STM32. _The old system was kept in the codebase for reverse compatibility and debugging purposes._

## Software Used

This code was prepared for TITAN's STM32s is written in C/C++ using the Arduino framework within the PlatformIO add-on for VS Code.


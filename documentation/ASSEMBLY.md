# Assembly of TITAN 2026

TITAN was once assembled, now it likely is no longer, so someone's got to put it back together. This will be written assuming TITAN's basically being entirely rebuilt.

## Lists of Parts

>[!NOTE]
> These lists do not account for any spare parts, those are up to your discretion.

There are a fair few parts to TITAN needed for operation that need to be packed listed below. Only one unit is needed of each unless specified otherwise.

- 2 x Raspberry Pis Model 3A+ or 3B+
- 2 x 16 GB micro SD cards (larger is fine but the excess storage will not be utilized)
- 2 x approximately 10" 720p HDMI display, 12&nbsp;V supply
- 2 x Raspberry Pi "Spy" camera
- TITAN 2026 HAT assembled for primary operation
- TITAN 2026 HAT assembled for secondary operation
- HDMI cable, 30&nbsp;cm long (for rear rider)
- GPS antenna with SMA connector on 3&nbsp;m cable
- 2.4 GHz antenna with 3&nbsp;m cable
- ANT+ reciever (USB-A)
- Analog pin hole video camera, 12&nbsp;V supply
- 7" analog input display, 12&nbsp;V supply

The following are needed but will likely be already integrated into TITAN's structure, so they don't need to be "packed".

- 3 x Battery extension leads, battery connector to XT-30F, 1.5&nbsp;m long
- 5.5&nbsp;mm barrel jack to barrel jack cable, 3&nbsp;m long
- 5.5&nbsp;mm barrel jack to XT-30F cable, 3&nbsp;m long
- Rear wheel sensor board with harness soldered on
- Brake disk temperature sensor for front wheel with USB-A connector
- USB-A to USB-A cable, 3&nbsp;m long
- HDMI cable, 3&nbsp;m long (for front rider)
- MH-Z19 CO2 sensor with 3&nbsp;m long cable
- 12&nbsp;V power distribution board
- RCA coax video cable, 3&nbsp;m long

## Setup Procedure

>[!CAUTION]
> **All systems must be entirely unpowered for setup!** Remove any and all batteries. The system was not designed for hot-plugging any component so damage may occur as a result.

This starts with the primary digital vision systems for each rider, then the backup system provided to the primary rider.

>[!TIP]
> These steps may be performed in any order when unpowered. Here they are presented in the order that will likely be the most convenient to perform.

1. Check the radio antennas are mounted to the radio window in the tail
   - nRF24 antenna should be posiitoned normal to the road
   - GPS antenna doesn't need a specific orientation
2. Check the sensors are installed
   - Front wheel brake temperature sensor in wheel bracket
   - Wheel sensor board in the rear wheel's bracket
   - MH-Z19 sensor between the heads of riders
3. Check that the auxiliary sensor connections are made
   - Front wheel sensor intermediate connector is mated near front yoke (might have been disconnected during maintenance or removal of yoke)
   - Front wheel to rear wheel cable harness connection
   - Camera cable extenders are properly securing the ribbon cables if used
   - Antenna cables are routed to electronics area
4. Image the micro SD cards for each RPi and insert them into the RPis
5. Connect the camera cables to each RPi, ensuring they are installed correctly by being perfectly normal to the board
6. Install the RPi HATs, mounted with threaded plastic spacer ideally
7. Insert the ANT+ receiver into the primary/front rider's RPi
8. Connect the sensor to the primary system's HAT paying attention that the sensors are located correctly
9. Connect the radio antennas to the appropriate connectors for the primary board
10. Connect the monitors to the RPis using HDMI cables for video and provide them power from same RPi's HAT
    - In TITAN there should be wires routed from the electronics area to the front rider's viewing area
    - Even the rear rider may require some extension cables to better position the electronics
11. Connect the RPi HATs with the bridge cable, from `SEC_OUT` on the secondary (rear rider's) HAT to `PRIM_IN` on the primary HAT
12. Connect the backup camera to the backup monitor using an RCA cable
13. Connect the backup camera and screen to the 12&nbsp;V power board

# TITAN Modifications from 2022

There were some hardware changes from 2022 to 2026, hese are all principly in the embedded "sensor" system which will need some changes to be made to the TITAN system. For a complete list of changes refer to the `README.md` files for the [main board](../hardware/titan_2026_rev_a/README.md#changes-from-2022) and [wheel board](../hardware/wheel_2026_rev_a/README.md#changes-from-2022) as this will summarize the ones that need to be acted on. These will be sorted into sections based on decending importance to complete.

>[!IMPORTANT]
> This does not cover the modifications and finally assembly needed for the boards themselves, refer to their configuration guides as needed. So far only the main board, the RPi HAT needs [additional hardware work](../hardware/titan_2026_rev_a/CONFIGURATION_AND_BODGES.md) before use.

## Bill of Materials

These are the materials needed to renovate the bike, excluding those to modify/fix anything on the new circuit boards which is covered in their respective guides.

- XT-30 cable connectors, at least 10 of both types
- JST-XH connector kit; with 3-, 5-, and 7-pin connector housings and crimps

## Mandatory Actions

These are actions that must be completed for the new hardware to be usable in TITAN, note that these will make it incompatible with earlier versions as a result.

- **Replace the HAT to HAT cable** this was a 4-pin cable but now will be a 7-pin JST-XH cable.
  - The cable should be a "crossover" cable, so pin 1 on one end goes to pin 7 on the other, and so on.
  - An easy check is if you plug both ends of the cable into the RPi HAT board, the connections should be adjacent to one another. What I mean is that the top connection on `PRIM_IN` should be the same as the top on `SEC_OUT`, and continuing for the length of the connector. If a ribbon cable is used, it should just fold over, but not need twisting to occupy both connectors.
- **Prepare the new 2026 RPi HAT boards** as per their [configuration guide](../hardware/titan_2026_rev_a/CONFIGURATION_AND_BODGES.md) and replace the 2022 HATs
- **Change the battery connectors** from barrel jacks to XT-30F for the RPis
- **Change the monitor power connectors** (to the HATs) from barrel jacks to XT-30M
- **Split the brake/wheel sensor cable harness Y** into a separate one for front and rear wheels
- **Crimp the front and rear wheel sensor cable to use a JST-XH** 5-pin connector abiding by the pinout on the PCB.
  - _Note: not all connections need to be used if not desired. E.g. the front wheel may not use the encoder._
- **Update the power scripts on the RPi** to reflect single button for on and off if using the SD card image from 2022.

## Optional Changes

These are changes that would be nice and improve TITAN in some way, but are not needed for the system to function.

- **Replace the brake disk sensor board from 2022 with the 2026 version.** The newer boards are almost identical electrically but should present far fewer chances for things to short out and be more reliable in the long run. Soldering the wires to pads can help ensure a low profile cable run preventing damage from riders.
- **Replace the USB-A intermediate wire-to-wire connections with locking connectors** to improve reliability against vibrations. The exact connector doesn't really matter as long as it has a locking feature. _This was a design choice made by me back in 2019 and has carried forth to today, I know much better now!_ If I am not mistaken this is only at the front fork.
- **Replace the four conductor cable used to connect the front wheel sensor with a five conductor one** so the front encoder can be used as well if the wheel board is installed there.
- **Get smaller batteries.** The ones we used in 2022 were 2200&nbsp;mAh or more 3S LiFePO4 if I recall correctly, this is far oversized and needless wight in my opinion. As mentioned in the [operation guide](./OPERATION.md#batteries-and-power), a _fully charged_ 1000&nbsp;mAh 3S LiFePO4 battery is sufficient to run any of the vision systems for around an hour which will suffice for a heat.

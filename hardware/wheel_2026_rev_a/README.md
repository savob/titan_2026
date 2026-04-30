# TITAN Wheel 2026 Rev. A

This board houses a brake temperature sensor as well as a wheel encoder for monitoring speed and distance. It was designed to fit within the wheel bracket close to the brake disk without interfering with the rider's pedaling.

## Changes from 2022

The main changes from the last version will be in improving physical reliability of the system

- Changed everything to other than sensors to be surface mount on the top of the PCB to eliminate the possibility of shorting against mounting surface
- Changed operational amplifier to a TLV2372IDR instead of an LM358 for better performance, namely rail-to-rail output
- Using pads to connect the data cables, headers can be soldered in later if desired
- Removed connector to daisy-chain wheels since that is now handled on the HAT

Looking on data from 2022, there was some noise on the encoder speed signal when plotted alongside the GPS derived speed. This could potentially benefit from filtering the signals, although I would really chalk it up to my firmware not properly capturing each spoke and not using a hardware timer.

## Tuning the Encoder's Comparator Operation

The onboard comparator circuit for the wheel encoder needs to have the two potentiometers to be properly set for reliable operation. One is responsible for the baseline threshold the encoder is compared to to know if the spoke is present of not, labeled `THRES`. The other potentiometer adjusts the level of hysteresis preventing spurious switching by moving the active threshold so the signal can't repeatedly cross it, labeled `HYST`.

To begin, the board must be installed into the wheel bracket and powered, _ideally via the TITAN HAT_. An oscilloscope is strongly recommended to probe the test pads while doing this to complete it accurately but, but not necessary. If possible, perform this with the wheel enclosed in its fairing - or out at least out of direct sunlight as it may influence the IR sensor.

### Without an Oscilloscope

1. Turn both potentiometers entirely clockwise to reset them
2. Rotate the wheel so that the IR sensor is entirely clear of any brake disk spokes
3. Check that the LED is lit up, if not there may be an issue with the board or too much ambient IR which will need different conditions for testing
4. Rotate the wheel so a spoke is directly in front of the IR sensor
5. Turn `THRES` counterclockwise to increase it until the LED turns off, remember this as the "low" mark
6. Spin the wheel and make sure that the LED is turning off with each passing of a spoke, and then back on again. Gradually increase `THRES` until the LED is no longer turning on again when the spoke is gone - this is the "high" mark.
7. Set `THRES` to be halfway between the "low" and "high" mark. Spinning the wheel should have the LED flash again with each spoke pass.
8. Keep spinning the wheel and increasing `HYST` by turning it counterclockwise
9. Once the LED is no longer changing state with the wheel's rotation, this it the "high" mark
10. Set `HYST` to be about halfway from the most clockwise point to the "high" mark
11. Spin the wheel to ensure that the LED is flashing again

This should roughly center the threshold correctly and provide a decent level of hysteresis. _It is impossible for the human eye to perceive the true effect of hysteresis with the LED alone, so we're basically just guessing this._

### With an Oscilloscope

The precise way to do this, no _major_ guesstimations here!

1. Turn both potentiometers entirely clockwise to "reset" them
2. Rotate the wheel so that the IR sensor is entirely clear of any brake disk spokes
3. Check that the LED is lit up, if not there may be an issue with the board or too much ambient IR which will need different conditions for testing
4. Read the voltage on the test point labeled `RAW`, this is our ambient level
5. Rotate the wheel so that there is a spoke directly in front of the IR sensor
6. Record the voltage on the test point labeled `RAW`, this is our presence level
7. Gradually turn `THRES` potentiometer counterclockwise until the voltage on the test point labeled `THR` is halfway between the ambient and presence levels. _If the spoke is still in front of the IR sensor at this point, the LED should turn off._
8. Spin the wheel to observe the LED turning on and off as the spoke passes
9. Keep the wheel spinning and probe `OUT`
10. Gradually increase `HYST` by turning the potentiometer counterclockwise until the signal on `OUT` shows a single pulse for each spoke pass, and then a sliver more - just not too much so that the LED stops lighting up
    - If you're also able to probe the other two signals you should be able to see that `THR` is changing between two levels and that `RAW` should be able to cross `THR` only once per spoke passing
    - `RAW` should continue to a level past `THR` after crossing it, so that even if `THR` didn't move `RAW` would not be causing potential switching with any sensor noise
    - If the hysteresis gets too large then it might make it hard to catch occasional pulses leading to slower perceived speed
11. Spin the wheel to ensure that the LED is flashing with each spoke passing

## Known Issues

There is a fundamental issue with the use of the IR sensor for the wheel encoder: the wheel encoder would fail to track the wheel speed once the brakes were engaged as it got saturated due to IR radiation from the brake disks reaching temperatures in the multi-hundred degrees as they stopped TITAN at the end of runs.

This issue was tolerable as we still had the GPS speed to fallback to, and the brakes would only be engaged when a run was ending so there was no longer a need to know TITAN's exact speed. **Future revisions should consider a different sensor type for the encoder, but this revision kept it since it was field proven.**

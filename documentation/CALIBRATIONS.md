# Calibration Protocols

TITAN uses some sensor systems that need calibration to operate at their best, so this documenta compiles the calibration procedure for those systems.

## Wheel Encoder

On the wheel board there is a circuit which uses an anlog IR reflectometer sensor to capture the passing of brake disk spokes. This sensor's analog output is passed through an analog comparator with a hysteresis circuit to be converted into a clean digital signal for the microcontroller to monitor. This section is larely copied from the board's own [`README.md](../hardware/wheel_2026_rev_a/README.md#tuning-the-encoders-comparator-operation).

To calibrate this circuit there are two potentiometers that need to be set: one for the base threshold the sensor's value is compared to, `THRES`, and then a second to increase the amount of hysteresis, `HYST`. Hysteresis is useful in preventing noise on the analog signal to cause multiple switches of the output fooling the system into thinking the wheel is spinning far faster than it is.

This calibration is best performed with the target wheel fully assembled and faired to best reflect the actual operating conditions, as ambient light does affect the sensor. **The only tool needed is a small flat head screwdriver for the potentiometers.** Although this calibration can be performed without any electrical tools, a multimeter or even oscilloscope will help greatly if available.

## Look Ma, No Electrical Tools

Instead of depending on electrical tools, one can complete the calibration with just the status LED present on the board

1. Connect the target wheel board to an RPi HAT board and supply power to the HAT board to power the sensor board - battery or USB power is fine.
2. _Gently_ turn both potentiometers entirely counterclockwise to reset them.
3. Rotate the wheel so that the IR sensor is entirely clear of any brake disk spokes.
4. Check that the LED is lit up, if not there may be an issue with the board or too much ambient IR which will need different conditions for testing.
5. Rotate the wheel so a spoke is directly in front of the IR sensor.
6. Turn `THRES` clockwise to increase it until the LED turns off, remember this as the "low" mark.
7. Spin the wheel and make sure that the LED is turning off with each passing of a spoke, and then back on again. Gradually increase `THRES` by turning it clockwise until the LED is no longer turning on again when the spoke is gone - this is the "high" mark.
8. Set `THRES` to be halfway between the "low" and "high" mark. Spinning the wheel should have the LED flash again with each spoke pass.
9. Keep spinning the wheel and increasing `HYST` by turning it clockwise.
10. Once the LED is no longer changing state with the wheel's rotation, this it the "high" mark.
11. Set `HYST` to be about halfway from the starting/zero point to the "high" mark.
12. Spin the wheel to ensure that the LED is flashing again.

This should roughly center the threshold correctly and provide a decent level of hysteresis. _It is impossible for the human eye to perceive the true effect of hysteresis with the LED alone, so we're basically just guessing this._ After checking with the LED it is nice to start TITAN properly and check that the speed readings shown seem reasonable and do not jump around wildly to accept the calibration.

## The Future is Now Old Man (Electrical Tools)

With some method of monitoring voltage in the system, either with a voltmeter or oscilloscope the process can be performed with more precision by using the solder pads left on the board along the top edge with a ground reference pad for convenience.

The precise way to do this, no _major_ guesstimations here!

1. Connect the target wheel board to an RPi HAT board and supply power to the HAT board to power the sensor board - battery or USB power is fine.
2. _Gently_ turn both potentiometers entirely counterclockwise to "reset" them.
3. Rotate the wheel so that the IR sensor is entirely clear of any brake disk spokes.
4. Check that the LED is lit up, if not there may be an issue with the board or too much ambient IR which will need different conditions for testing.
5. Read the voltage on the test point labeled `RAW`, this is our ambient level.
6. Rotate the wheel so that there is a spoke directly in front of the IR sensor.
7. Record the voltage on the test point labeled `RAW`, this is our presence level.
8. Gradually turn `THRES` potentiometer until the voltage on the test point labeled `THR` is halfway between the ambient and presence levels. _If the spoke is still in front of the IR sensor at this point, the LED should turn off._
9. Spin the wheel to observe the LED turning on and off as the spoke passes.
10. Keep the wheel spinning and probe `OUT`.
11. Gradually increase `HYST` by turning the potentiometer clockwise until the signal on `OUT` shows a single pulse for each spoke pass rather than a series of pulses during the transition. Then turn `HYST` a sliver more - just not too much so that the `OUT` stops switching
    - If you're also able to probe the other two signals you should be able to see that `THR` is changing between two levels and that `RAW` should be able to cross `THR` only once per spoke passing.
    - `RAW` should continue to a level past `THR` after crossing it, so that even if `THR` didn't move `RAW` would not be causing potential switching with any sensor noise.
    - If the hysteresis gets too large then it might make it hard to catch occasional pulses leading to slower perceived speed.
12. Spin the wheel to ensure that the LED is flashing with each spoke passing.

If done correctly with measurements, especially an oscillscope, then one can be fairly confident in the result of this calibration. Nonetheless, it is still worthwihle to start up TITAN's video system and see the values shown on the overlay match expectations and don't fluctuate wildly.

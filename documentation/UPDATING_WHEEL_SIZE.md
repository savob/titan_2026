# Updating Wheel Size

If there are any changes to the _(effective)_ wheel circumference of TITAN they should be reflected properly in the code to ensure we are still calculating speeds and distances correctly from our wheel encoders.

Unfortunately this system inherits a bit of the sloppy design choices I made in 2022 and maybe even as far back as 2019, one such symptom is that we need to update the wheel raduis in multiple places because for some reason I thought it was best to communicate distance in terms of completed rotations rather than a calculated value. If I were to do it again I would only have it set in the microcontroller and provide distance directly.

Luckily the changes are pretty easy to administer in either system. When performing these updates, they should be done for all boards at a time to ensure none are left with the old value - _even the spares!_

## Updating the Microcontroller

The microcontroller has the value hardcoded as `CIRCUMFERENCE_M` in `update_wheel_status()` in `main.c`, simply set it to properly reflect the active circumference in metres.

It may also be necessay to update the number of spokes on the brake disk in this function if the brakedisk is changed.

```c
inline static void update_wheel_status(volatile struct WheelStatus* status, uint32_t capture_period_count) {
    const float CIRCUMFERENCE_M = 2.136;
    const uint8_t SPOKES = 6; // The number of spokes (counts) per complete wheel rotation

    ...
}
```

Once updated you need to program the microcontroller as described in the [firmware `README.md`](../firmware/README.md#programming-the-boards).

## Updating the Video System

In the overlay system the circumference is defined as a global constant `WHEEL_CIRCUMFERENCE_M` at the top of the `main.c` file

```c
const float WHEEL_CIRCUMFERENCE_M = 2.136;
```

Once changed, the overlay code will need to be recompiled into a new binary for the system as described in the [vision system `README.md`](../vision/README.md#compiling-the-overlay-code).

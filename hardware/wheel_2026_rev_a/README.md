# TITAN Wheel 2026 Rev. A

This board houses a brake temperature sensor as well as a wheel encoder for monitoring speed and distance. It was designed to fit within the wheel structure close to the brake disk without interfering with the rider's pedalling.

## Changes from 2022

I plan to move away from an IR-based encoder for monitoring wheel speed, and spend some time trying to improve filtering. An issue found during operation was that the wheel encoder would fail to track the wheel speed once the brakes were engaged as it got saturated when the brake disks reached the multi-hundred degrees as they stopped TITAN at the end of runs.

The cable connector was also prone to damage from the rider's passing pedal, so a lower profile connector system would be useful, although it should be easy to maintain by hand and not depend on a specialized connector so field repairs are easy when they inevitably arrive.

Furthermore there should be a move to using only surface mount parts on the top of the PCB to prevent the board from short circuiting when installed in the structure.

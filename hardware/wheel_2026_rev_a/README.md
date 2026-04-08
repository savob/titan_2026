# TITAN Wheel 2026 Rev. A

This board houses a brake temperature sensor as well as a wheel encoder for monitoring speed and distance. It was designed to fit within the wheel structure close to the brake disk without interfering with the rider's pedalling.

## Known Issues

An issue found during operation was that the wheel encoder would fail to track the wheel speed once the brakes were engaged. We realized this was because the encoder used an infrared reflection sensor sor it got overwhelmed when the brake disks reached the multi-hundred degrees as they stopped TITAN at the end of runs. So moving away from IR to another encoder sensor is strongly recommended. There were also some issues regarding measurement noise even without thermal issues.

The cable connector was also prone to damage from the rider's passing pedal, so a lower profile connector system would be useful, although it should be easy to maintain by hand and not depend on a specialized connector so field repairs are easy when they inevitably arrive.

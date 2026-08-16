# Working with ANT+ Devices

The ANT+ protocol is used for providing some data from biometric device: the power pedals and heart rate monitors. It's largely being phased out for BLE but we'll stay comfortably in the past.

## Setup

To properly perform the operations needed to use ANT devices you will need some ANT capable receiver and one of their development tools. This can take the form of either:

- ANT USB stick connected to a Windows PC runing [Simulant](https://www.thisisant.com/resources/simulant-1). _You will need to make a free account on their site._
- An Android phone with the [ANT Plugin Sampler App](https://play.google.com/store/apps/details?id=com.dsi.ant.antplus.pluginsampler&hl=en) installed. _This will likely require an old Android phone since many have started dropping ANT support in favour of BLE._

## Recording Expected USB Dongle

The TITAN vision system performs a check on boot to see if an ANT receiver stick is present to determine if it is the primary/front system (ANT present) or not. It does this by checking all the connected USB devices for a vendor and product number (VID/PID) to match what it expects.

>[!IMPORTANT]
> The expected USB dongle must be correctly identified or else no ANT data will be collected for the rider overlays.

This "magic" value it is looking for is recorded in hexadecimal (with lowercase letters) as `ANTStickID` in `titan_startup.py` under `vision/`. An example value is `0fcf:1008` for the currently expected USB stick.

To get the VID/PID of an ANT receiver the [methods differ based on system](https://superuser.com/a/1106248). In either case start by connecting the ANT receiver to your computer.

### Windows USB ID Gathering

Go to "Device manager", find your device, right click on it, select "Properties", go to "Details" tab, select "Hardware IDs" from the drop-down, and you will find an entry of a form:

```terminal
HID\VID_046D&PID_C05A 
```

which are correspondingly vendor and product IDs, i.e. `046d:c05a`

### Linux / RPi USB ID Gathering

Run the following command from your terminal to see pairs of `VID:PID` near the names of all of the USB devices connected.

```bash
lsusb
```

## Pairing Devices

Each ANT device has three device identifying fields that are needed to ensure one connects to the right device. These are the following which are generally 16-bit integers.

- Device number
- Device type
- Transmission type

To acquire this information from a device, one needs to pair with it while using one of the ANT developer tools and record the values reported. These values then need to be copied as appropriate into the `titanant.py` script used under `vision/`.

>[!NOTE]
> There is no need to pair your ANT device to the receiver. Once their identifying information is recorded in the scripts, they will have the ANT receiver search and pair with the devices automatically whenever the system starts.

## Configuring ANT Devices

In TITAN we only use heart rate monitors and torque sensing bicycle pedal ANT devices. Heart rate monitors require no configuration to work properly once paired with a device.

**Torque sensing pedals require a little bit of configuration to properly work**. If I recall correctly, they only need the pedal crank length to be programmed so they properly measure torque and thus power. This can be acheived through the ANT development software mentioned at the start of this file, I personally found using Simulant on PC the easiest.

>[!TIP]
> Once any device is configured it is wise to reboot it and check if the values they report are reasonable through the ANT development tools.

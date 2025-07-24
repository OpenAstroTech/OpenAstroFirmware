# Requirements for Firmware running a mount

## Time/Date/Location
- Must be aware of date, time and location in order to calculate HA
- Support for Northern and Southern hemisphere

## RA axis
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop
- Must run at a constant speed without changing coordinate (tracking)
- Must be able to run faster or slower for some duration (with millisecond accuracy) without changing coordinate (guiding)
- Must be able to have new coordinate sync'd from connected software
- All these operations must be done without losing home position
- Should support homing operations with a sensor to home on

## DEC axis
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop
- Must be able to run forward or reverse for some duration (with millisecond accuracy) without changing coordinate (guiding)
- Must be able to have new coordinate sync'd from connected software
- All these operations must be done without losing home position
- Should support homing operations with a sensor to home on

## ALT/AZ
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop
- Should support configurable min/max positions and limit axis movement accordingly
- Should save home and current position to EEPROM

## Focuser
- Must be able to slew at various rates until requested to stop
- Must support slewing to a specific target position
- Must be able to set motor position

## General
- RA and DEC operations must be independent, but can overlap
- Stepper operations must be async
- Would be nice to get an event when the stepper stops
- Serial message processing must be sequential, but async and in the background
- Serial connection default speed should be 115200
- Must be able to convert from RA/DEC coordinates to stepper target positions (must use solution with longest safe time)
- Meade Protocol support for OAT/OAM/OAE
- Should be able to return safe time left
- Should be able to return time to meridian for current position

## Menu/Display
- Support Info-only displays to show mount status
- Support LCD screens (small, large, color, mono), but maybe start with common/standardized size
- Should support input devices like Buttons, Joysticks, Dial with press point, Touch screen
- Need to come up with a list of operations possible from interactive display devices
  - Move any axes in any directions at various speeds
  - Start/stop tracking
  - Find home position 
  - Go to Home position
  - Enter target coordinates and slew there

## Sensors
- Support for RA and DEC end switches
- Support for RA/DEC homing switches
- Temperature sensor?

## Improvements from 1.0
- Support meridian flip. This should already be supported in apps like NINA, if the sync to coordinate takes the longest safe time.
- Dew heater support?

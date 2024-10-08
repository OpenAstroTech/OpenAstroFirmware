# Requirements for Firmware running a mount

## Time/Date/Location
- Must be aware of date, time and location in order to calculate HA
- Support for Northern and Southern hemisphere

## RA axis
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop
- Must run at a constant speed without changing coordinate (tracking)
- Must be able to run faster or slower for some duration without changing coordinate (guiding)
- Must be able to have new coordinate sync'd 
- All these operations must be done without losing home position

## DEC axis
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop
- Must be able to run forward or reverse for some duration without changing coordinate (guiding)
- Must be able to have new coordinate sync'd 
- All these operations must be done without losing home position

## ALT/AZ
- Must be able to slew by a given number of degrees
- Must be able to slew at various rates until requested to stop

## Focuser
- Must be able to slew at various rates until requested to stop

## General
- RA and DEC operations must be independent
- Stepper operations must be async
- Would be nice to get an event when the stepper stops
- Serial message processing is sequential, but async and in the background
- Must be able to convert from RA/DEC coordinates to stepper target positions (must use solution with longest safe time)
- Meade Protocol support for OAT/OAM/OAE

## Sensors
- Support for RA and DEC end switches
- Support for RA/DEC homing switches
- Temperature sensor?

## Improvements from 1.0
- Support meridian flip? (how is this done when using camera in another app?)
- Dew heater support?

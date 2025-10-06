# Meade Telescope Serial Command Protocol

**Revision L (Meade LX200) + OpenAstroTech Extensions**  
*Original: 9 October 2002*  
*OAT Extensions: Current as of Firmware V1.13.12*

*Source Reference: [OpenAstroTracker-Firmware](https://github.com/OpenAstroTech/OpenAstroTracker-Firmware/blob/main/src/MeadeCommandProcessor.cpp)*

> **Note**: This documentation is manually synchronized with the OpenAstroTracker firmware implementation.
> The firmware is the authoritative source for command behavior.

## Introduction

This document combines two protocol specifications:

1. **Meade LX200 Serial Control Protocol** - The industry-standard protocol for telescope control, compatible with ASCOM, INDI, N.I.N.A., and SkySafari
2. **OpenAstroTech (OAT) Extensions** - Additional commands specific to the OpenAstroTracker and OpenAstroMount firmware

The Meade protocol contains a core of common commands supported by all telescopes. Due to different implementations and technological advances, some commands are not supported by all models. The differences are noted in the descriptive text.

As an extension to the Telescope Protocol beginning with the LX200GPS, a possible response to any command is ASCII NAK (0x15). Should the telescope control chain be busy and unable to accept an process the command, a NAK will be sent within 10 msec of the receipt of the '#' terminating the command. In this event, the controller should wait a reasonable interval and retry the command.

**OpenAstroTech Note**: OAT firmware implements a subset of Meade LX200 commands plus extensive OAT-specific extensions (primarily using the `:X` prefix). These extensions provide advanced features like Hall sensor homing, digital level support, backlash compensation, and detailed mount status queries.

## Command Support Matrix

| Command Designator Symbol | Command Group | AutoStar | LX200<16" | LX 16" | LX200GPS | OAT |
|---------------------------|---------------|----------|-----------|--------|----------|-----|
| `<ACK>` | Alignment Query | x | p | p | x | - |
| A | Alignment* | x | - | p | x | - |
| $B | Active Backlash | - | - | - | x | - |
| B | Reticule Control* | p | p | - | p | - |
| C | Sync Control | x | x | p | - | x |
| D | Distance Bars | p | p | x | p | x |
| f | Fan* | - | - | x | x | - |
| F | Focus Control Commands | x | x | x | x | x |
| g | GPS Commands | - | - | - | x | x |
| G | Get Information | x | x | x | x | x |
| h | Home Position Commands* | - | p | x | x | x |
| H | Hour | p | p | x | x | - |
| I | Initialize Telescope | - | - | p | p | x |
| L | Library | x | x | x | x | - |
| M | Movement | x | x | x | x | x |
| P | High Precision | p | p | x | x | - |
| $Q | Smart Drive Control* | - | - | p | - | - |
| Q | Quit Command | x | x | p | p | x |
| r | Field De-rotator | - | - | x | - | - |
| R | Rate Control | x | x | x | x | x |
| S | Set Information | x | x | x | x | x |
| T | Tracking Frequency | x | x | x | x | - |
| U | User Format Control | p | p | x | x | - |
| W | Way point (Site) | - | - | p | p | - |
| X | **OAT Extensions** | - | - | - | - | **x** |
| ? | Help Commands | - | - | x | x | - |

**Notes:**
- Commands accepted by the telescopes are shown in the table above indicated by an "x" entry. This means that the telescope will accept these commands and respond with a syntactically valid response where required.
- A "p" indicated only a subset of this command class is supported. Due to the differing implementations of the telescopes, some of the commands may provide static responses or may do nothing in response to the command. See the detailed description of the commands below to determine the exact behavior.
- **OAT column**: Indicates support in OpenAstroTracker/OpenAstroMount firmware V1.13.12+
- **X commands**: Extensive OAT-specific extensions for advanced mount control (50+ commands)

## Command Details

### ACK - Alignment Query

#### `:ACK` <0x06> Query of alignment mounting mode.

Returns:
- `A` - If scope in AltAz Mode
- `L` - If scope in Land Mode
- `P` - If scope in Polar Mode

### A - Alignment Commands

#### `:Aa#` Start Telescope Automatic Alignment Sequence [LX200GPS only]

Returns:
- `1` - When complete (can take several minutes).
- `0` - If scope not AzEl Mounted or align fails

#### `:AL#` Sets telescope to Land alignment mode

Returns: nothing

#### `:AP#` Sets telescope to Polar alignment mode

Returns: nothing

#### `:AA#` Sets telescope the AltAz alignment mode

Returns: nothing

### $B – Active Backlash Compensation

#### `:$BAdd#` Set Altitude/Dec Antibacklash

Returns Nothing

#### `:$BZdd#` Set Azimuth/RA Antibacklash

Returns Nothing

### B - Reticule/Accessory Control

#### `:B+#` Increase reticule Brightness

Return: Nothing

#### `:B-#` Decrease Reticule Brightness

Return: Nothing

#### `:B<n>#` Set Reticle flash rate to <n> (an ASCII expressed number)

- `<n>` Values of 0..3 for LX200 series
- `<n>` Values of 0..9 for Autostar and LX200GPS

Return: Nothing

#### `:BDn#` Set Reticule Duty flash duty cycle to <n> (an ASCII expressed digit) [LX200 GPS Only]

Return: Nothing

- `<n>` Values: 0 = On, 1..15 flash rate

### C - Sync Control

#### `:CL#` Synchronize the telescope with the current Selenographic coordinates.

#### `:CM#` Synchronizes the telescope's position with the currently selected database object's coordinates.

Returns:
- LX200's - a "#" terminated string with the name of the object that was synced.
- Autostars & LX200GPS - At static string: " M31 EX GAL MAG 3.5 SZ178.0'#"

### D - Distance Bars

#### `:D#` Requests a string of bars indicating the distance to the current library object.

Returns:
- LX200's – a string of bar characters indicating the distance.
- Autostars and LX200GPS – a string containing one bar until a slew is complete, then a null string is returned.

### f - Fan Command

#### `:f+#` Turn on fan/power
- LX 16"– Turn on the tube exhaust fan
- LX200GPS – Turn on power to accessor panel
- Autostar & LX200 < 16" – Not Supported

Returns: nothing

#### `:f-#` Turn off fan/power
- LX 16"– Turn off tube exhaust fan
- LX200GPS - Turn off power to accessory panel
- Autostar & LX200 < 16" – Not Supported

Returns: Nothing

#### `:fT#` Return temperature [LX200GPS only]

LX200GPS – Return Optical Tube Assembly Temperature

Returns `<sdd.ddd>#` - a '#' terminated signed ASCII real number indicating the Celsius ambient temperature.

All others – Not supported

### F – Focuser Control

#### `:F+#` Start Focuser moving inward (toward objective)

Returns: None

#### `:F-#` Start Focuser moving outward (away from objective)

Returns: None

#### `:FQ#` Halt Focuser Motion

Returns: Nothing

#### `:FF#` Set Focus speed to fastest setting

Returns: Nothing

#### `:FS#` Set Focus speed to slowest setting

Returns: Nothing

#### `:F<n>#` Set focuser speed to <n> where <n> is an ASCII digit 1..4

- Autostar & LX200GPS – set focuser speed to <n> where <n> is an ASCII digit 1..4
- LX200 – Not Supported

Returns: Nothing

### g – GPS/Magnetometer commands

#### `:g+#` Turn on GPS [LX200GPS Only]

Returns: Nothing

#### `:g-#` Turn off GPS [LX200GPS Only]

Returns: Nothing

#### `:gps#` Turns on NMEA GPS data stream [LX200GPS Only]

Returns: The next string from the GPS in standard NEMA format followed by a '#' key

#### `:gT#` Update system time from GPS [LX200GPS only]

Powers up the GPS and updates the system time from the GPS stream. The process my take several minutes to complete. During GPS update, normal handbox operations are interrupted.

Returns:
- `0` - In the event that the user interrupts the process, or the GPS times out.
- `1` - After successful updates

### G – Get Telescope Information

#### `:G0#` Get Alignment Menu Entry 0 [LX200 legacy command]

Returns: A '#' Terminated ASCII string.

#### `:G1#` Get Alignment Menu Entry 1 [LX200 legacy command]

Returns: A '#' Terminated ASCII string.

#### `:G2#` Get Alignment Menu Entry 2 [LX200 legacy command]

Returns: A '#' Terminated ASCII string.

#### `:GA#` Get Telescope Altitude

Returns: `sDD*MM#` or `sDD*MM'SS#`

The current scope altitude. The returned format depending on the current precision setting.

#### `:Ga#` Get Local Telescope Time In 12 Hour Format

Returns: `HH:MM:SS#`

The time in 12 format

#### `:Gb#` Get Browse Brighter Magnitude Limit

Returns: `sMM.M#`

The magnitude of the faintest object to be returned from the telescope FIND/BROWSE command when searching for objects in the Deep Sky database.

#### `:GC#` Get current date.

Returns: `MM/DD/YY#`

The current local calendar date for the telescope.

#### `:Gc#` Get Calendar Format

Returns: `12#` or `24#`

Depending on the current telescope format setting.

#### `:GD#` Get Telescope Declination.

Returns: `sDD*MM#` or `sDD*MM'SS#`

Depending upon the current precision setting for the telescope.

#### `:Gd#` Get Currently Selected Object/Target Declination

Returns: `sDD*MM#` or `sDD*MM'SS#`

Depending upon the current precision setting for the telescope.

#### `:GF#` Get Find Field Diameter

Returns: `NNN#`

An ASCII integer expressing the diameter of the field search used in the IDENTIFY/FIND commands.

#### `:Gf#` Get Browse Faint Magnitude Limit

Returns: `sMM.M#`

The magnitude or the brightest object to be returned from the telescope FIND/BROWSE command.

#### `:GG#` Get UTC offset time

Returns: `sHH#` or `sHH.H#`

The number of decimal hours to add to local time to convert it to UTC. If the number is a whole number the `sHH#` form is returned, otherwise the longer form is return. On Autostar and LX200GPS, the daylight savings setting in effect is factored into returned value.

#### `:Gg#` Get Current Site Longitude

Returns: `sDDD*MM#`

The current site Longitude. East Longitudes are expressed as negative

#### `:Gh#` Get High Limit

Returns: `sDD*`

The minimum elevation of an object above the horizon to which the telescope will slew with reporting a "Below Horizon" error.

#### `:GL#` Get Local Time in 24 hour format

Returns: `HH:MM:SS#`

The Local Time in 24-hour Format

#### `:Gl#` Get Larger Size Limit

Returns: `NNN'#`

The size of the smallest object to be returned by a search of the telescope using the BROWSE/FIND commands.

#### `:GM#` Get Site 1 Name

Returns: `<string>#`

A '#' terminated string with the name of the requested site.

#### `:GN#` Get Site 2 Name

Returns: `<string>#`

A '#' terminated string with the name of the requested site.

#### `:GO#` Get Site 3 Name

Returns: `<string>#`

A '#' terminated string with the name of the requested site.

#### `:GP#` Get Site 4 Name

Returns: `<string>#`

A '#' terminated string with the name of the requested site.

#### `:Go#` Get Lower Limit

Returns: `DD*#`

The highest elevation above the horizon that the telescope will be allowed to slew to without a warning message.

#### `:Gq#` Get Minimum Quality For Find Operation

Returns:
- `SU#` - Super
- `EX#` - Excellent
- `VG#` - Very Good
- `GD#` - Good
- `FR#` - Fair
- `PR#` - Poor
- `VP#` - Very Poor

The minimum quality of object returned by the FIND command.

#### `:GR#` Get Telescope RA

Returns: `HH:MM.T#` or `HH:MM:SS#`

Depending which precision is set for the telescope

#### `:Gr#` Get current/target object RA

Returns: `HH:MM.T#` or `HH:MM:SS`

Depending upon which precision is set for the telescope

#### `:GS#` Get the Sidereal Time

Returns: `HH:MM:SS#`

The Sidereal Time as an ASCII Sexidecimal value in 24 hour format

#### `:Gs#` Get Smaller Size Limit

Returns: `NNN'#`

The size of the largest object returned by the FIND command expressed in arcminutes.

#### `:GT#` Get tracking rate

Returns: `TT.T#`

Current Track Frequency expressed in hertz assuming a synchronous motor design where a 60.0 Hz motor clock would produce 1 revolution of the telescope in 24 hours.

#### `:Gt#` Get Current Site Latitude

Returns: `sDD*MM#`

The latitude of the current site. Positive implies North latitude.

#### `:GVD#` Get Telescope Firmware Date

Returns: `mmm dd yyyy#`

#### `:GVN#` Get Telescope Firmware Number

Returns: `dd.d#`

#### `:GVP#` Get Telescope Product Name

Returns: `<string>#`

#### `:GVT#` Get Telescope Firmware Time

Returns: `HH:MM:SS#`

#### `:Gy#` Get deepsky object search string

Returns: `GPDCO#`

A string indicating the class of objects that should be returned by the FIND/BROWSE command. If the character is upper case, the object class is return. If the character is lowercase, objects of this class are ignored. The character meanings are as follows:
- G – Galaxies
- P – Planetary Nebulas
- D – Diffuse Nebulas
- C – Globular Clusters
- O – Open Clusters

#### `:GZ#` Get telescope azimuth

Returns: `DDD*MM#T` or `DDD*MM'SS#`

The current telescope Azimuth depending on the selected precision.

### h – Home Position Commands

#### `:hS#` Seek and store home position

LX200GPS and LX 16" Seeks Home Position and stores the encoder values from the aligned telescope at the home position in the nonvolatile memory of the scope.

Returns: Nothing

Autostar,LX200 – Ignored

#### `:hF#` Seek home position and align

LX200GPS and LX 16" Seeks the Home Position of the scope and sets/aligns the scope based on the encoder values stored in non-volatile memory

Returns: Nothing

Autostar,LX200 - Ignored

#### `:hN#` Sleep Telescope [LX200GPS only]

Power off motors, encoders, displays and lights. Scope remains in minimum power mode until a keystroke is received or a wake command is sent.

#### `:hP#` Slew to Park Position

Autostar, LX200GPS and LX 16" Slew to Park Position

Returns: Nothing

#### `:hW#` Wake up sleeping telescope [LX200 GPS Only]

#### `:h?#` Query Home Status

Autostar, LX200GPS and LX 16" Query Home Status

Returns:
- `0` - Home Search Failed
- `1` - Home Search Found
- `2` - Home Search in Progress

LX200 Not Supported

### H – Time Format Command

#### `:H#` Toggle Between 24 and 12 hour time format

Returns: Nothing

### I – Initialize Telescope Command

#### `:I#` Initialize telescope [LX200 GPS Only]

Causes the telescope to cease current operations and restart at its power on initialization.

### L – Object Library Commands

#### `:LB#` Find previous object and set it as the current target object.

Returns: Nothing

LX200GPS & Autostar – Performs no function

#### `:LCNNNN#` Set current target object to deep sky catalog object number NNNN

Returns: Nothing

LX200GPS & Autostar – Implemented in later firmware revisions

#### `:LF#` Find Object using current constraints

Find Object using the current Size, Type, Upper limit, lower limit and Quality constraints and set it as current target object.

Returns: Nothing

LX200GPS & Autostar – Performs no function

#### `:Lf#` Identify object in current field

Returns: `<string>#`

Where the string contains the number of objects in field & object in center field.

LX200GPS & Autostar – Performs no function. Returns static string "0 - Objects found".

#### `:LI#` Get Object Information

Returns: `<string>#`

Returns a string containing the current target object's name and object type.

LX200GPS & Autostar – performs no operation. Returns static description of Andromeda Galaxy.

#### `:LMNNNN#` Set current target object to Messier Object NNNN

Set current target object to Messier Object NNNN, an ASCII expressed decimal number.

Returns: Nothing.

LX200GPS and Autostar – Implemented in later versions.

#### `:LN#` Find next deep sky target object

Find next deep sky target object subject to the current constraints.

LX200GPS & AutoStar – Performs no function

#### `:LoD#` Select deep sky Library where D specifies

- 0 - Objects CNGC / NGC in Autostar & LX200GPS
- 1 - Objects IC
- 2 – UGC
- 3 – Caldwell (Autostar & LX200GPS)
- 4 – Arp (LX200 GPS)
- 5 – Abell (LX200 GPS)

Returns:
- `1` - Catalog available
- `0` - Catalog Not found

LX200GPS & AutoStar – Performs no function always returns "1"

#### `:LsD#` Select star catalog D

Select star catalog D, an ASCII integer where D specifies:
- 0 - STAR library (Not supported on Autostar I & II)
- 1 - SAO library
- 2 - GCVS library
- 3 - Hipparcos (Autostar I & 2)
- 4 - HR (Autostar I & 2)
- 5 - HD (Autostar I & 2)

Returns:
- `1` - Catalog Available
- `2` - Catalog Not Found

#### `:LSNNNN#` Select star NNNN as the current target object

Select star NNNN as the current target object from the currently selected catalog

Returns: Nothing

LX200GPS & AutoStar – Available in later firmwares

### M – Telescope Movement Commands

#### `:MA#` Slew to target Alt and Az

Autostar, LX 16", LX200GPS – Slew to target Alt and Az

Returns:
- `0` - No fault
- `1` - Fault

LX200 – Not supported

#### `:Me#` Move Telescope East at current slew rate

Returns: Nothing

#### `:Mn#` Move Telescope North at current slew rate

Returns: Nothing

#### `:Ms#` Move Telescope South at current slew rate

Returns: Nothing

#### `:Mw#` Move Telescope West at current slew rate

Returns: Nothing

#### `:MS#` Slew to Target Object

Returns:
- `0` - Slew is Possible
- `1<string>#` - Object Below Horizon w/string message
- `2<string>#` - Object Below Higher w/string message

### P - High Precision Toggle

#### `:P#` Toggle High Precision Pointing

Toggles High Precision Pointing. When High precision pointing is enabled scope will first allow the operator to center a nearby bright star before moving to the actual target.

Returns: `<string>`
- `"HIGH PRECISION"` - Current setting after this command.
- `"LOW PRECISION"` - Current setting after this command.

### $Q – Smart Drive Control

#### `:$Q#` Toggle Smart Drive PEC on and off for both axis

Returns: Nothing

Not supported on Autostar

#### `:$QA+` Enable Dec/Alt PEC [LX200GPS only]

Returns: Nothing

#### `:$QA-` Disable Dec/Alt PEC [LX200GPS only]

Returns: Nothing

#### `:$QZ+` Enable RA/AZ PEC compensation [LX200GPS only]

Returns: Nothing

#### `:$QZ-` Disable RA/AZ PEC Compensation [LX200GPS only]

Return: Nothing

### Q – Movement Commands

#### `:Q#` Halt all current slewing

Returns: Nothing

#### `:Qe#` Halt eastward Slews

Returns: Nothing

#### `:Qn#` Halt northward Slews

Returns: Nothing

#### `:Qs#` Halt southward Slews

Returns: Nothing

#### `:Qw#` Halt westward Slews

Returns: Nothing

#### `:Qa#` Halt all direction Slews [OAT Extension]

Halts slewing in all directions (equivalent to stopping n, s, e, w individually).

Returns: Nothing

### r – Field Derotator Commands

#### `:r+#` Turn on Field Derotator [LX 16" and LX200GPS]

Returns: Nothing

#### `:r-#` Turn off Field Derotator [LX 16" and LX200GPS]

Turn off Field Derotator, halt slew in progress.

Returns Nothing

### R – Slew Rate Commands

#### `:RC#` Set Slew rate to Centering rate (2nd slowest)

Returns: Nothing

#### `:RG#` Set Slew rate to Guiding Rate (slowest)

Returns: Nothing

#### `:RM#` Set Slew rate to Find Rate (2nd Fastest)

Returns: Nothing

#### `:RS#` Set Slew rate to max (fastest)

Returns: Nothing

#### `:RADD.D#` Set RA/Azimuth Slew rate [LX200GPS Only]

Set RA/Azimuth Slew rate to DD.D degrees per second

Returns: Nothing

#### `:REDD.D#` Set Dec/Elevation Slew rate [LX200GPS only]

Set Dec/Elevation Slew rate to DD.D degrees per second

Returns: Nothing

#### `:RgSS.S#` Set guide rate [LX200GPS only]

Set guide rate to +/- SS.S to arc seconds per second. This rate is added to or subtracted from the current tracking Rates when the CCD guider or handbox guider buttons are pressed when the guide rate is selected. Rate shall not exceed sidereal speed (approx 15.0417"/sec)

Returns: Nothing

### S – Telescope Set Commands

#### `:SasDD*MM#` Set target object altitude

Set target object altitude to sDD*MM# or sDD*MM'SS# [LX 16", Autostar, LX200GPS]

Returns:
- `0` - Object within slew range
- `1` - Object out of slew range

#### `:SbsMM.M#` Set Brighter limit

Set Brighter limit to the ASCII decimal magnitude string. SMM.M

Returns:
- `0` - Valid
- `1` - Invalid number

#### `:SBn#` Set Baud Rate n

Set Baud Rate n, where n is an ASCII digit (1..9) with the following interpretation:
- 1 - 56.7K
- 2 - 38.4K
- 3 - 28.8K
- 4 - 19.2K
- 5 - 14.4K
- 6 - 9600
- 7 - 4800
- 8 - 2400
- 9 - 1200

Returns: `1` - At the current baud rate and then changes to the new rate for further communication

#### `:SCMM/DD/YY#` Change Handbox Date

Change Handbox Date to MM/DD/YY

Returns: `<D><string>`
- D = '0' if the date is invalid. The string is the null string.
- D = '1' for valid dates and the string is "Updating Planetary Data#                                           #"

Note: For LX200GPS this is the UTC data!

#### `:SdsDD*MM#` Set target object declination

Set target object declination to sDD*MM or sDD*MM:SS depending on the current precision setting

Returns:
- `1` - Dec Accepted
- `0` - Dec invalid

#### `:SEsDD*MM#` Set selenographic latitude

Sets target object to the specified selenographic latitude on the Moon.

Returns:
- `1` - If moon is up and coordinates are accepted.
- `0` - If the coordinates are invalid

#### `:SesDDD*MM#` Set selenographic longitude

Sets the target object to the specified selenographic longitude on the Moon

Returns:
- `1` - If the Moon is up and coordinates are accepted.
- `0` - If the coordinates are invalid for any reason.

#### `:SfsMM.M#` Set faint magnitude limit

Set faint magnitude limit to sMM.M

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SFNNN#` Set FIELD/IDENTIFY field diameter

Set FIELD/IDENTIFY field diameter to NNNN arc minutes.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SgDDD*MM#` Set current site's longitude

Set current site's longitude to DDD*MM an ASCII position string

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SGsHH.H#` Set UTC offset

Set the number of hours added to local time to yield UTC

Returns:
- `0` - Invalid
- `1` - Valid

#### `:ShDD#` Set minimum object elevation limit

Set the minimum object elevation limit to DD#

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SlNNN#` Set smallest object size limit

Set the size of the smallest object returned by FIND/BROWSE to NNNN arc minutes

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SLHH:MM:SS#` Set the local Time

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SM<string>#` Set site 1's name

Set site 1's name to be `<string>`. LX200s only accept 3 character strings. Other scopes accept up to 15 characters.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SN<string>#` Set site 2's name

Set site 2's name to be `<string>`. LX200s only accept 3 character strings. Other scopes accept up to 15 characters.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SO<string>#` Set site 3's name

Set site 3's name to be `<string>`. LX200s only accept 3 character strings. Other scopes accept up to 15 characters.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SP<string>#` Set site 4's name

Set site 4's name to be `<string>`. LX200s only accept 3 character strings. Other scopes accept up to 15 characters.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SoDD*#` Set highest elevation limit

Set highest elevation to which the telescope will slew

Returns:
- `0` - Invalid
- `1` - Valid

#### `:Sq#` Step quality limit

Step the quality of limit used in FIND/BROWSE through its cycle of VP … SU. Current setting can be queried with `:Gq#`

Returns: Nothing

#### `:SrHH:MM.T#` or `:SrHH:MM:SS#` Set target object RA

Set target object RA to HH:MM.T or HH:MM:SS depending on the current precision setting.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SsNNN#` Set largest object size limit

Set the size of the largest object the FIND/BROWSE command will return to NNNN arc minutes

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SSHH:MM:SS#` Set local sidereal time

Sets the local sidereal time to HH:MM:SS

Returns:
- `0` - Invalid
- `1` - Valid

#### `:StsDD*MM#` Set current site latitude

Sets the current site latitude to sDD*MM#

Returns:
- `0` - Invalid
- `1` - Valid

#### `:STTT.T#` Set tracking rate

Sets the current tracking rate to TTT.T hertz, assuming a model where a 60.0 Hertz synchronous motor will cause the RA axis to make exactly one revolution in 24 hours.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SwN#` Set maximum slew rate

Set maximum slew rate to N degrees per second. N is the range (2..8)

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SyGPDCO#` Set object selection string

Sets the object selection string used by the FIND/BROWSE command.

Returns:
- `0` - Invalid
- `1` - Valid

#### `:SzDDD*MM#` Set target Object Azimuth

Sets the target Object Azimuth [LX 16" and LX200GPS only]

Returns:
- `0` - Invalid
- `1` - Valid

### T – Tracking Commands

#### `:T+#` Increment Manual rate by 0.1 Hz

Returns: Nothing

#### `:T-#` Decrement Manual rate by 0.1 Hz

Returns: Nothing

#### `:TL#` Set Lunar Tracking Rate

Returns: Nothing

#### `:TM#` Select custom tracking rate

Returns: Nothing

#### `:TQ#` Select default tracking rate

Returns: Nothing

#### `:TDDD.DDD#` Set Manual rate

Set Manual rate do the ASCII expressed decimal DDD.DD

Returns: `1`

### U - Precision Toggle

#### `:U#` Toggle between low/hi precision positions

- Low - RA displays and messages HH:MM.T sDD*MM
- High - Dec/Az/El displays and messages HH:MM:SS sDD*MM:SS

Returns Nothing

### W – Site Select

#### `:W<n>#` Set current site

Set current site to `<n>`, an ASCII digit in the range 0..3

Returns: Nothing

### ? – Help Text Retrieval

#### `:??#` Set help text cursor to start

Set help text cursor to the start of the first line.

Returns: `<string>#`

The `<string>` contains first string of the general handbox help file.

#### `:?+#` Retrieve the next line of help text

Returns: `<string>#`

The `<string>` contains the next string of general handbox help file

#### `:?-#` Retrieve previous line of help text

Retrieve previous line of the handbox help text file.

Returns: `<string>#`

The `<string>` contains the next string of general handbox help file

## Appendix A: LX200GPS Command Extensions

The following commands are extensions specific to the LX200GPS:

| Command | Description |
|---------|-------------|
| `:Aa#` | Automatically align scope |
| `:$BAdd#` | Set Altitude/Dec Antibacklash |
| `:$BZdd#` | Set Azimuth/RA Antibacklash |
| `:BD<n>#` | Programmable Reticule Duty Cycle |
| `:F<n>#` | Set Focuer Speed |
| `:g+#` | Turn on GPS power |
| `:g-#` | Turn off GPS power |
| `:gps#` | Stream GPS data |
| `:gT#` | Updates Time of Day from GPS |
| `:I#` | Initialize Telescope |
| `:$QZ+#` | RA PEC Enable |
| `:$QZ-#` | RA PEC Disable |
| `:$QA+#` | Dec PEC Enable |
| `:$QA-#` | Dec PEC Disable |
| `:RADD.D#` | Programmable Slew Rates |
| `:REDD.D#` | Programmable Slew Rates |
| `:RgSS.S#` | Programmable Guiding Rates |
| `:SBn#` | Set Baud Rate |

## Appendix B: OpenAstroTech Extensions

**Current as of Firmware V1.13.12**

OpenAstroTech firmware implements a superset of the Meade LX200 protocol with extensive custom extensions. These extensions provide advanced features for DIY telescope mount control.

### B.1 - OAT Sync Control (Enhanced)

#### `:CM#` Synchronize Declination and Right Ascension

This tells the scope what it is currently pointing at. The scope synchronizes to the current target coordinates.

Returns: `NONE#`

**Note**: Set target coordinates first with `:Sd#` and `:Sr#`

#### `:SYsDD*MM:SS.HH:MM:SS#` Synchronize to Exact Coordinates [OAT Extension]

This tells the scope the exact coordinates it is currently pointing at. These coordinates become the new current RA/DEC coordinates of the mount.

Returns:
- `1` - If successfully set
- `0` - Otherwise

Parameters:
- `s` - Sign (+ or -)
- `DD` - Degrees
- `MM` - Minutes
- `SS` - Seconds
- `HH` - Hours

### B.2 - OAT GPS Commands

**Note**: `:gT#` in OAT differs from Meade LX200GPS implementation.

#### `:gT#` Set Mount Time from GPS [OAT]

Attempts to set the mount time and location from GPS for 2 minutes. This is a **blocking call** - no other activities take place (except tracking if interrupt-driven).

Returns:
- `1` - If the data was set
- `0` - If not (timed out)

**Note**: Use `:Gt#` and `:Gg#` to retrieve Lat and Long after successful GPS sync.

#### `:gTnnn#` Set Mount Time with Timeout [OAT Extension]

Attempts to set the mount time and location from GPS with a custom timeout. This is also blocking but by using a low timeout, you can avoid long pauses.

Returns:
- `1` - If the data was set
- `0` - If not (timed out)

Parameters:
- `nnn` - Integer defining the number of milliseconds to wait for GPS to get a bearing

### B.3 - OAT Get Extensions

#### `:GIS#` Get DEC or RA Slewing Status [OAT Extension]

Returns:
- `1#` - If either RA or DEC is slewing
- `0#` - If not

#### `:GIT#` Get Tracking Status [OAT Extension]

Returns:
- `1#` - If tracking is on
- `0#` - If not

#### `:GIG#` Get Guiding Status [OAT Extension]

Returns:
- `1#` - If currently guiding
- `0#` - If not

#### `:GX#` Get Mount Status [OAT Extension]

Returns detailed mount status as comma-delimited string.

Returns: `Idle,--T--,11219,0,927,071906,+900000,,#`

Parameters (comma-separated):
- [0] Mount status: 'Idle', 'Parked', 'Parking', 'Guiding', 'SlewToTarget', 'FreeSlew', 'ManualSlew', 'Tracking', 'Homing'
- [1] Motion state (6 characters, see below)
- [2] RA stepper position
- [3] DEC stepper position
- [4] Tracking stepper position
- [5] Current RA coordinate
- [6] Current DEC coordinate
- [7] FOC stepper position (if FOC enabled, else empty)

**Motion State Characters**:
- Position 1: RA slewing ('R' = East, 'r' = West, '-' = stopped)
- Position 2: DEC slewing ('d' = North, 'D' = South, '-' = stopped)
- Position 3: TRK slewing ('T' = Tracking, '-' = stopped)
- Position 4: AZ slewing ('Z' or 'z' = adjusting, '-' = stopped)
- Position 5: ALT slewing ('A' or 'a' = adjusting, '-' = stopped)
- Position 6: FOC slewing ('F' or 'f' = adjusting, '-' = stopped)

### B.4 - OAT Movement Extensions

#### `:MGdnnnn#` Run Guide Pulse [OAT Extension]

Runs the RA or DEC steppers at an increased/decreased speed (RA) or constant speed (DEC) for a short period. Used for autoguiding.

Returns: `1`

Parameters:
- `d` - Direction: 'N', 'E', 'W', or 'S'
- `nnnn` - Duration in milliseconds

#### `:MTs#` Set Tracking Mode [OAT Extension]

Turns the scope's tracking mode on or off.

Returns: `1`

Parameters:
- `s` - `1` to turn on Tracking, `0` to turn it off

#### `:Mc#` Start Slewing [OAT Extension]

Starts slewing the mount in the given direction. Must issue a stop command (`:Qc#` where 'c' is the same direction, or `:Q#` to stop all) to stop it.

Returns: Nothing

Parameters:
- `c` - Direction: 'n', 'e', 'w', or 's'

#### `:MXxnnnnn#` Move Stepper [OAT Extension]

Moves one of the steppers by the given number of steps and returns immediately. Steps can be positive or negative.

Returns:
- `1` - If successfully scheduled
- `0` - Otherwise

Parameters:
- `x` - Stepper to move: 'r' (RA), 'd' (DEC), 'f' (FOC), 'z' (AZ), 't' (ALT)
- `nnnnn` - Number of steps (signed integer)

#### `:MHRxn#` Home RA Stepper via Hall Sensor [OAT Extension]

Attempts to find the Hall sensor and home the RA ring accordingly.

Returns:
- `1` - If search is started
- `0` - If homing has not been enabled in config

Parameters:
- `x` - Direction to start search: 'R' (CCW) or 'L' (CW)
- `n` - (Optional) Maximum degrees to move while searching (5-75°, default 30°)

**Behavior**:
1. Moves up to 30° (or specified) in initial direction
2. If no sensor found, moves 60° (2x) in opposite direction
3. If sensor found, centers on trigger range then applies home offset (`:XSHRnnnn#`)
4. If sensor already triggered, moves off trigger (max 15°) before searching

#### `:MHDxn#` Home DEC Stepper via Hall Sensor [OAT Extension]

Attempts to find the Hall sensor and home the DEC axis accordingly.

Returns:
- `1` - If search is started
- `0` - If homing has not been enabled in config

Parameters:
- `x` - Direction to start search: 'U' (up) or 'D' (down)
- `n` - (Optional) Maximum degrees to move while searching (5-75°, default 30°)

**Behavior**: Same as `:MHR#` but for DEC axis.

#### `:MAZn.nn#` Move Azimuth [OAT Extension]

If the scope supports automated azimuth operation, move azimuth by n.nn arcminutes.

Returns: Nothing

Parameters:
- `n.nn` - Signed floating point number representing arcminutes to move left/right

#### `:MALn.nn#` Move Altitude [OAT Extension]

If the scope supports automated altitude operation, move altitude by n.nn arcminutes.

Returns: Nothing

Parameters:
- `n.nn` - Signed floating point number representing arcminutes to raise/lower

#### `:MAAH#` Move Azimuth and Altitude to Home [OAT Extension]

If the scope supports automated azimuth and altitude operations, move AZ and ALT axes to their zero positions.

Returns: `1`

### B.5 - OAT Set Extensions

#### `:SHHH:MM#` Set HA (Hour Angle of Polaris) [OAT Extension]

Sets the scope's HA, which should be that of Polaris.

Returns:
- `1` - If successfully set
- `0` - Otherwise

Parameters:
- `HH` - Hours
- `MM` - Minutes

#### `:SHP#` Set Home Point [OAT Extension]

Sets the current orientation of the scope as its home point.

Returns: `1`

#### `:SHLHH:MM#` Set LST Time [OAT Extension]

Sets the scope's LST (and HA).

Returns:
- `1` - If successfully set
- `0` - Otherwise

Parameters:
- `HH` - Hours
- `MM` - Minutes

### B.6 - OAT Home/Park Extensions

#### `:hU#` Unpark Scope [OAT Extension]

Unparks the scope (currently simply turns on tracking).

Returns: `1`

#### `:hZ#` Set Home Position for AZ and ALT Axes [OAT Extension]

If the mount supports AZ and ALT axes, this sets their positions to 0 and stores in persistent storage.

Returns: `1`

### B.7 - OAT Quit Extensions

#### `:Qq#` Disconnect and Quit Control Mode [OAT Extension]

Quits Serial Control mode and starts tracking.

Returns: Nothing

### B.8 - OAT Focus Extensions

#### `:Fp#` Get Focuser Position [OAT Extension]

Get the current position of the focus stepper motor.

Returns: `nnn#` where `nnn` is the current position

#### `:FPnnn#` Set Focuser Position [OAT Extension]

Sets the current position of the focus stepper motor (does not move stepper).

Returns: `1`

Parameters:
- `nnn` - New position of the stepper

#### `:FB#` Get Focuser State [OAT Extension]

Gets the state of the focuser stepper.

Returns:
- `0` - Focuser is idle
- `1` - Focuser is moving

### B.9 - OAT Extra Commands (X Family)

The X command family provides extensive OAT-specific functionality used by the OATControl PC application.

#### `:XFR#` Perform Factory Reset [OAT Extension]

Clears all EEPROM settings.

Returns: `1#`

#### `:XDnnn#` Run Drift Alignment [OAT Extension]

Runs a drift alignment procedure where the mount slews east, pauses, slews west and pauses. This is a **blocking call**.

Returns: Nothing

Parameters:
- `nnn` - Number of seconds the entire alignment should take

**Note**: Only supported if `SUPPORT_DRIFT_ALIGNMENT` is enabled in firmware.

#### `:XL0#` / `:XL1#` Digital Level Control [OAT Extension]

Turn digital level off (`:XL0#`) or on (`:XL1#`).

Returns:
- `1#` - If successful
- `0#` - If there is no Digital Level

#### `:XLGR#` Get Digital Level Reference [OAT Extension]

Gets the reference pitch and roll values (values when mount is level).

Returns:
- `<pitch>,<roll>#` - If Digital Level present
- `0#` - If no Digital Level

#### `:XLGC#` Get Digital Level Current Values [OAT Extension]

Gets the current pitch and roll values.

Returns:
- `<pitch>,<roll>#` - If Digital Level present
- `0#` - If no Digital Level

#### `:XLGT#` Get Digital Level Temperature [OAT Extension]

Get current temperature in Celsius.

Returns:
- `<temp>#` - If Digital Level present
- `0#` - If no Digital Level

#### `:XLSR#` / `:XLSP#` Set Digital Level Reference [OAT Extension]

Sets the reference roll (`:XLSR#`) or pitch (`:XLSP#`) value (value at which mount is level).

Returns:
- `1#` - If successful
- `0#` - If no Digital Level

#### `:XGAA#` Get AZ and ALT Positions [OAT Extension]

Get current position in steps of AZ and ALT axes.

Returns: `azpos|altpos#` (returns 0 for disabled axes)

#### `:XGAH#` Get Auto Homing State [OAT Extension]

Get current state of RA and DEC autohoming status.

Returns:
- `rastate|decstate#` - If either axis enabled
- `|#` - If no autohoming enabled

**States During Homing**: `MOVE_OFF`, `MOVING_OFF`, `STOP_AT_TIME`, `WAIT_FOR_STOP`, `START_FIND_START`, `FINDING_START`, `FINDING_START_REVERSE`, `FINDING_END`, `RANGE_FOUND`

#### `:XGB#` Get Backlash Correction Steps [OAT Extension]

Get the number of steps the RA stepper needs to overshoot and backtrack when slewing east.

Returns: `integer#`

#### `:XGCn.nn*m.mm#` Get Stepper Positions for Target [OAT Extension]

Get the positions of stepper motors when pointed at given coordinates.

Returns: `ralong,declong#`

Parameters:
- `n.nn` - RA coordinate (0.0 - 23.999)
- `m.mm` - DEC coordinate (-90.00 - +90.00)

#### `:XGR#` / `:XGD#` Get Steps per Degree [OAT Extension]

Get the number of steps per degree for RA (`:XGR#`) or DEC (`:XGD#`) stepper.

Returns: `float#`

#### `:XGDLx#` Get DEC Limits [OAT Extension]

Get lower, upper, or both limits for DEC stepper in degrees.

Returns:
- `float#` - If x is 'U' (upper) or 'L' (lower)
- `float|float#` - If x is omitted (both limits)

Parameters:
- `x` - Optional parameter: 'U' for upper limit only, 'L' for lower limit only, omit for both

#### `:XGDP#` Get DEC Parking Position [OAT Extension] **OBSOLETE/DISABLED**

Gets the number of steps from the home position to the parking position for DEC.

Returns: `0#`

**Note**: This command is obsolete and disabled in current firmware. It always returns 0.

#### `:XGS#` Get Tracking Speed Adjustment [OAT Extension]

Get the adjustment factor used to speed up (>1.0) or slow down (<1.0) tracking speed.

Returns: `float#`

#### `:XGST#` Get Remaining Safe Time [OAT Extension]

Get the number of hours before the RA ring reaches its end.

Returns: `float#`

#### `:XGT#` Get Tracking Speed [OAT Extension]

Get the absolute tracking speed of the mount.

Returns: `float#`

#### `:XGH#` Get HA (Hour Angle of Polaris) [OAT Extension]

Get the current HA of Polaris that the mount thinks it is.

Returns: `HHMMSS#`

#### `:XGHR#` / `:XGHD#` Get Homing Offset [OAT Extension]

Get the RA (`:XGHR#`) or DEC (`:XGHD#`) ring homing offset in steps from Hall sensor center.

Returns: `n#` - Number of steps

#### `:XGHS#` Get Hemisphere [OAT Extension]

Get the hemisphere that OAT currently assumes it is operating in (set via latitude).

Returns:
- `N#` - Northern hemisphere
- `S#` - Southern hemisphere

#### `:XGM#` Get Mount Configuration Settings [OAT Extension]

Returns comprehensive mount configuration.

Returns: `<board>,<RA Stepper Info>,<DEC Stepper Info>,<GPS info>,<AzAlt info>,<Gyro info>,<Display info>,<Focuser info>,<RAHallSensor info>,<Endswitch info>#`

Parameters:
- `<board>` - Mega, ESP32, or MKS
- `<Stepper Info>` - Pipe-delimited: Motor type|Pulley Teeth|Steps per revolution
- `<GPS info>` - NO_GPS or GPS
- `<AzAlt info>` - NO_AZ_ALT, AUTO_AZ_ALT, AUTO_AZ, or AUTO_ALT
- `<Gyro info>` - NO_GYRO or GYRO
- `<Display info>` - NO_LCD or LCD_display_type
- `<Focuser info>` - NO_FOC or FOC
- `<RAHallSensor info>` - NO_HSAH or HSAH
- `<Endswitch info>` - NO_ENDSW, ENDS_RA, ENDSW_DEC, or ENDSW_RA_DEC

Example: `ESP32,28BYJ|16|4096.00,28BYJ|16|4096.00,NO_GPS,NO_AZ_ALT,NO_GYRO,NO_LCD,NO_FOC,NO_ENDSW#`

#### `:XGMS#` Get Mount Driver Configuration [OAT Extension]

Returns driver configuration for RA and DEC.

Returns: `<RA driver>,<RA slewMS>,<RA trackMS>|<DEC driver>,<DEC slewMS>,<DEC guideMS>|#`

Parameters:
- `<driver>` - TU (TMC2209UART), TS (TMC2209STANDALONE), A (A4983)
- `<slewMS>` - Microstepping divider when slewing (1, 2, 4, 8, 15, 21, 64, 128, 256)
- `<trackMS>` - Microstepping divider when tracking RA
- `<guideMS>` - Microstepping divider when guiding DEC

Example: `TU,8,64|TU,16,64|#`

#### `:XGN#` Get Network Settings [OAT Extension]

Gets current WiFi connection status (ESP boards only).

Returns:
- `1,<mode>,<status>,<hostname>,<ip>:<port>,<SSID>,<OATHostname>#` - If WiFi enabled
- `0,#` - If WiFi not enabled

#### `:XGL#` Get LST [OAT Extension]

Get the current LST of the mount.

Returns: `HHMMSS#`

#### `:XGO#` Get Log Buffer [OAT Extension]

Get the current debug log buffer contents. This is a debugging command used for troubleshooting.

Returns: `<log_content>#`

**Note**: This is a debugging command primarily used for development and troubleshooting.

#### `:XSBn#` Set Backlash Correction Steps [OAT Extension]

Sets the number of steps the RA stepper needs to overshoot and backtrack when slewing east.

Returns: Nothing

Parameters:
- `n` - Number of steps

#### `:XSHRnnn#` / `:XSHDnnn#` Set Homing Offset [OAT Extension]

Set the RA (`:XSHR#`) or DEC (`:XSHD#`) ring homing offset from Hall sensor center.

Returns: Nothing

Parameters:
- `nnn` - Positive or negative number of steps from Hall sensor center to actual home position

#### `:XSRn.n#` / `:XSDn.n#` Set Steps per Degree [OAT Extension]

Set the number of steps per degree for RA (`:XSR#`) or DEC (`:XSD#`) stepper.

Returns: Nothing

Parameters:
- `n.n` - Number of steps (only one decimal point supported, must be positive)

#### `:XSDLUnnnnn#` Set DEC Upper Limit [OAT Extension]

Set the upper limit for DEC axis.

Returns: Nothing

Parameters:
- `nnnnn` - (Optional) Number of steps from home. Omit to use current position. Pass 0 to reset to config default.

#### `:XSDLu#` Clear DEC Upper Limit [OAT Extension]

Resets the upper limit for DEC axis to configuration-defined position.

Returns: Nothing

#### `:XSDLLnnnnn#` Set DEC Lower Limit [OAT Extension]

Set the lower limit for DEC axis.

Returns: Nothing

Parameters:
- `nnnnn` - (Optional) Number of steps from home. Omit to use current position. Pass 0 to reset to config default.

#### `:XSDLl#` Clear DEC Lower Limit [OAT Extension]

Resets the lower limit for DEC axis to configuration-defined position. If not configured, the limit is cleared.

Returns: Nothing

#### `:XSDPnnnn#` Set DEC Parking Position Offset [OAT Extension] **OBSOLETE/DISABLED**

This stores the number of steps needed to move from home to the parking position.

Returns: Nothing

Parameters:
- `nnnn` - Number of steps from home to parking position

**Note**: This command is obsolete and disabled in current firmware. Calling it has no effect.

#### `:XSSn.nnn#` Set Tracking Speed Adjustment [OAT Extension]

Set the adjustment factor to speed up (>1.0) or slow down (<1.0) tracking speed.

Returns: Nothing

Parameters:
- `n.nnn` - Factor to multiply theoretical speed by

#### `:XSTnnnn#` Set Tracking Motor Position [OAT Extension]

**Debugging aid only** - sets internal tracking steps to given value without movement.

Returns: Nothing

Parameters:
- `nnnn` - Stepper steps to set

⚠️ **Warning**: Not recommended unless you know what you're doing.

#### `:XSMn#` Set Manual Slewing Mode [OAT Extension]

Toggle manual slewing mode where RA and DEC motors run at constant speed.

Returns: Nothing

Parameters:
- `n` - '1' to turn on, otherwise off

#### `:XSXn.nnn#` / `:XSYn.nnn#` Set Manual Slewing Speed [OAT Extension]

Set RA (`:XSX#`) or DEC (`:XSY#`) manual slewing speed in degrees/sec immediately. Max ~2.5 deg/s.

Returns: Nothing

Parameters:
- `n.nnn` - Speed in degrees per second

**Note**: Must be in manual slewing mode first (`:XSM1#`).

---

## Appendix C: OAT vs Meade Differences

### Key Behavioral Differences

1. **`:gT#` Command**:
   - **Meade LX200GPS**: Updates time from GPS, returns after GPS lock or user interrupt
   - **OAT**: Same behavior but supports timeout parameter (`:gTnnn#`)

2. **`:CM#` Command**:
   - **Meade**: Returns object name string
   - **OAT**: Returns `NONE#`

3. **`:D#` Command**:
   - **Meade**: Returns distance bars to library object
   - **OAT**: Returns mount slewing status (`|#` if slewing, `#` if not)

4. **Focus Commands**:
   - **OAT Adds**: `:Fp#` (get position), `:FP#` (set position), `:FB#` (get state)

5. **Movement Commands**:
   - **OAT Adds**: Extensive extensions for guide pulses, tracking control, stepper control, Hall sensor homing

### Implementation Notes for Developers

- OAT firmware is open source and available at: https://github.com/OpenAstroTech
- Protocol documentation is auto-generated from firmware source
- The `:X` family commands are OAT-specific and not part of the Meade specification
- OAT implements a **subset** of Meade commands - not all Meade commands are supported
- For Arduino/ESP32-based DIY telescope mounts, use OAT protocol
- For commercial Meade telescopes, use standard Meade protocol only

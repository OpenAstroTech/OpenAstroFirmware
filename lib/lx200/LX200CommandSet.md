# Meade Telescope Serial Command Protocol

**Revision L**  
*9 October 2002*

## Introduction

This paper documents the Meade Telescope Serial Control Protocol utilized to remotely command and control Meade Telescopes. This command language contains a core of common commands supported by all telescope. Due to different implementation and technological advances the command has extension that are not supported by all model. The differences are noted in the descriptive text for the commands. Finally, there are a series of new commands proposed for the LX200GPS. These commands are indicated in the Appendix A at the end of this document.

As an extension to the Telescope Protocol beginning with the LX200GPS, a possible response to any command is ASCII NAK (0x15). Should the telescope control chain be busy and unable to accept an process the command, a NAK will be sent within 10 msec of the receipt of the '#' terminating the command. In this event, the controller should wait a reasonable interval and retry the command.

## Command Support Matrix

| Command Designator Symbol | Command Group | AutoStar | LX200<16" | LX 16" | LX200GPS |
|---------------------------|---------------|----------|-----------|--------|----------|
| `<ACK>` | Alignment Query | x | p | p | x |
| A | Alignment* | x | - | p | x |
| $B | Active Backlash | - | - | - | x |
| B | Reticule Control* | p | p | - | p |
| C | Sync Control | x | x | p | - |
| D | Distance Bars | p | p | x | p |
| f | Fan* | - | - | x | x |
| F | Focus Control Commands | x | x | x | x |
| g | GPS Commands | - | - | - | x |
| G | Get Information | x | x | x | x |
| h | Home Position Commands* | - | p | x | x |
| H | Hour | p | p | x | x |
| I | Initialize Telescope | - | - | p | p |
| L | Library | x | x | x | x |
| M | Movement | x | x | x | x |
| P | High Precision | p | p | x | x |
| $Q | Smart Drive Control* | - | - | p | - |
| Q | Quit Command | x | x | p | p |
| r | Field De-rotator | - | - | x | - |
| R | Rate Control | x | x | x | x |
| S | Set Information | x | x | x | x |
| T | Tracking Frequency | x | x | x | x |
| U | User Format Control | p | p | x | x |
| W | Way point (Site) | - | - | p | p |
| ? | Help Commands | - | - | x | x |

**Notes:**
- Commands accepted by the telescopes are shown in the table above indicated by an "x" entry. This means that the telescope will accept these commands and respond with a syntactically valid response where required.
- A "p" indicated only a subset of this command class is supported. Due to the differing implementations of the telescopes, some of the commands may provide static responses or may do nothing in response to the command. See the detailed description of the commands below to determine the exact behavior.

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

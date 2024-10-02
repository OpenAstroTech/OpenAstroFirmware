#ifndef FIRMWARE_LIB_LX200_H
#define FIRMWARE_LIB_LX200_H

/* Alignment Commands */
// :Aa#    Start Telescope Automatic Alignment Sequence [LX200GPS only]
// :AL#    Sets telescope to Land alignment mode
// :AP#    Sets telescope to Polar alignment mode
// :AA#    Sets telescope the AltAz alignment mode

/* Active Backlash Compensation */
// :$BAdd# Set Altitude/Dec Antibacklash
// :$BZdd# Set Azimuth/RA Antibacklash

/* Reticule/Accessory Control */
// :B+#    Increase reticule Brightness
// :B-#    Decrease Reticule Brightness
// :B<n>#  Set Reticle flash rate to <n> (an ASCII expressed number)
// :BDn#   Set Reticule Duty flash duty cycle to <n> (an ASCII expressed digit) [LX200 GPS Only]

/* Sync Control */
// :CL#    Synchonize the telescope with the current Selenographic coordinates.
// :CM#    Synchronizes the telescope's position with the currently selected database object's coordinates.

/* Distance Bars */
// :D#     Requests a string of bars indicating the distance to the current library object.

/* Fan Command */
// :f+#    LX 16”– Turn on the tube exhaust fan, LX200GPS – Turn on power to accessor panel
// :f-#    LX 16”– Turn off tube exhaust fan, LX200GPS - Turn off power to accessory panel
// :fT#    LX200GPS – Return Optical Tube Assembly Temperature

/* Focuser Control */
// :F+#    Start Focuser moving inward (toward objective)
// :F-#    Start Focuser moving outward (away from objective)
// :FQ#    Halt Focuser Motion
// :FF#    Set Focus speed to fastest setting
// :FS#    Set Focus speed to slowest setting
// :F<n>#  Autostar & LX200GPS – set focuser speed to <n> where <n> is an ASCII digit 1..4

/* GPS/Magnetometer commands */
// :g+#    LX200GPS Only - Turn on GPS
// :g-#    LX200GPS Only - Turn off GPS
// :gps#   LX200GPS Only – Turns on NMEA GPS data stream.
// :gT#    Powers up the GPS and updates the system time from the GPS stream.

/* Get Telescope Information */
// :G0#    Get Alignment Menu Entry 0 [LX200 legacy command]
// :G1#    Get Alignment Menu Entry 0 [LX200 legacy command]
// :G2#    Get Alignment Menu Entry 0 [LX200 legacy command]
// :GA#    Get Telescope Altitude
// :Ga#    Get Local Telescope Time In 12 Hour Format
// :Gb#    Get Browse Brighter Magnitude Limit
// :GC#    Get current date.
// :Gc#    Get Calendar Format
// :GD#    Get Telescope Declination.
// :Gd#    Get Currently Selected Object/Target Declination
// :GF#    Get Find Field Diameter
// :Gf#    Get Browse Faint Magnitude Limit
// :GG#    Get UTC offset time
// :Gg#    Get Current Site Longitude
// :Gh#    Get High Limit
// :GL#    Get Local Time in 24 hour format
// :Gl#    Get Larger Size Limit
// :GM#    Get Site 1 Name
// :GN#    Get Site 2 Name
// :GO#    Get Site 3 Name
// :GP#    Get Site 4 Name
// :Go#    Get Lower Limit
// :Gq#    Get Minimum Quality For Find Operation
// :GR#    Get Telescope RA
// :Gr#    Get current/target object RA
// :GS#    Get the Sidereal Time
// :Gs#    Get Smaller Size Limit
// :GT#    Get tracking rate
// :Gt#    Get Current Site Latitude
// :GVD#   Get Telescope Firmware Date
// :GVN#   Get Telescope Firmware Number
// :GVP#   Get Telescope Product Name
// :GVT#   Get Telescope Firmware Time
// :Gy#    Get deepsky object search string
// :GZ#    Get telescope azimuth

/* Home Position Commands */
// :hS#    LX200GPS and LX 16” Seeks Home Position and stores the encoder values
// :hF#    LX200GPS and LX 16” Seeks the Home Position of the scope and sets/aligns
// :hN#    LX200GPS only: Sleep Telescope.
// :hP#    Autostar, LX200GPS and LX 16”Slew to Park Position
// :hW#    LX200 GPS Only: Wake up sleeping telescope.
// :h?#    Autostar, LX200GPS and LX 16” Query Home Status

/* Time Format Command */
// :H#     Toggle Between 24 and 12 hour time format

/* Initialize Telescope Command */
// :I#     LX200 GPS Only - Causes the telescope to cease current operations and restart at its power on initialization.

/* Object Library Commands */
// :LB#    Find previous object and set it as the current target object.
// :LCNNNN# Set current target object to deep sky catalog object number NNNN
// :LF#    Find Object using the current Size, Type, Upper limit, lower limit and Quality constraints and set it as current target object.
// :Lf#    Identify object in current field.
// :LI#    Get Object Information
// :LMNNNN# Set current target object to Messier Object NNNN, an ASCII expressed decimal number.
// :LN#    Find next deep sky target object subject to the current constraints.
// :LoD#   Select deep sky Library where D specifies
// :LsD#   Select star catalog D, an ASCII integer where D specifies
// :LSNNNN# Select star NNNN as the current target object from the currently selected catalog

/* Telescope Movement Commands */
// :MA#    Autostar, LX 16”, LX200GPS – Slew to target Alt and Az
// :Me#    Move Telescope East at current slew rate
// :Mn#    Move Telescope North at current slew rate
// :Ms#    Move Telescope South at current slew rate
// :Mw#    Move Telescope West at current slew rate
// :MS#    Slew to Target Object

/* High Precision Toggle */
// :P#     Toggles High Precision Pointing.

/* Smart Drive Control */
// $Q#     Toggles Smart Drive PEC on and off for both axis
// :$QA+   Enable Dec/Alt PEC [LX200gps only]
// :$QA-   Enable Dec/Alt PEC [LX200gps only]
// :$QZ+   Enable RA/AZ PEC compensation [LX200gps only]
// :$QZ-   Disable RA/AZ PEC Compensation [LX200gps only]

/* Movement Commands */
// :Q#     Halt all current slewing
// :Qe#    Halt eastward Slews
// :Qn#    Halt northward Slews
// :Qs#    Halt southward Slews
// :Qw#    Halt westward Slews

/* Field Derotator Commands */
// :r+#    Turn on Field Derotator [LX 16” and LX200GPS]
// :r-#    Turn off Field Derotator, halt slew in progress. [Lx 16” and LX200GPS]

/* Slew Rate Commands */
// :RC#    Set Slew rate to Centering rate (2nd slowest)
// :RG#    Set Slew rate to Guiding Rate (slowest)
// :RM#    Set Slew rate to Find Rate (2nd Fastest)
// :RS#    Set Slew rate to max (fastest)
// :RADD.D# Set RA/Azimuth Slew rate to DD.D degrees per second [LX200GPS Only]
// :REDD.D# Set Dec/Elevation Slew rate to DD.D degrees per second [LX200GPS only]
// :RgSS.S# Set guide rate to +/- SS.S to arc seconds per second.

/* Telescope Set Commands */
// :SasDD*MM# Set target object altitude to sDD*MM# or sDD*MM’SS# [LX 16”, Autostar, LX200GPS]
// :SbsMM.M#  Set Brighter limit to the ASCII decimal magnitude string. SMM.M
// :SBn#      Set Baud Rate n, where n is an ASCII digit (1..9)
// :SCMM/DD/YY# Change Handbox Date to MM/DD/YY
// :SdsDD*MM# Set target object declination to sDD*MM

// meade lx200 command handler
typedef struct
{
    /**
     * @brief LX200 GPS Only - Causes the telescope to cease current operations and restart at its power on initialization.
     */
    void (*lx200_initialize)();
} lx200_command_handler_t;

/**
 * @brief Parse the command and call the appropriate handler function
 */
void lx200_parse_command(char *command, int cnt, lx200_command_handler_t *handler);

#endif
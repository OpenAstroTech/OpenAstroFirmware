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

#include <string>

class MeadeLX200Parser
{
public:
    MeadeLX200Parser();
    ~MeadeLX200Parser();

    /**
     * @brief Process a lx200 command string.
     *
     * @param command The command string to process. E.g: ":I#"
     */
    void processCommand(std::string command);

protected:
    // Alignment Commands
    /**
     * :Aa#    Start Telescope Automatic Alignment Sequence [LX200GPS only]
     */
    virtual void handleAutomaticAlignment();

    /**
     * :AL#    Sets telescope to Land alignment mode
     */
    virtual void handleLandAlignmentMode();

    /**
     * :AP#    Sets telescope to Polar alignment mode
     */
    virtual void handlePolarAlignmentMode();

    /**
     * :AA#    Sets telescope the AltAz alignment mode
     */
    virtual void handleAltAzAlignmentMode();

    // Active Backlash Compensation
    /**
     * :$BAdd# Set Altitude/Dec Antibacklash
     */
    virtual void handleSetAltitudeDecAntibacklash();

    /**
     * :$BZdd# Set Azimuth/RA Antibacklash
     */
    virtual void handleSetAzimuthRAAntibacklash();

    // Reticule/Accessory Control
    /**
     * :B+#    Increase reticule Brightness
     */
    virtual void handleIncreaseReticuleBrightness();

    /**
     * :B-#    Decrease Reticule Brightness
     */
    virtual void handleDecreaseReticuleBrightness();

    /**
     * :B<n>#  Set Reticle flash rate to <n> (an ASCII expressed number)
     */
    virtual void handleSetReticleFlashRate(int n);

    /**
     * :BDn#   Set Reticule Duty flash duty cycle to <n> (an ASCII expressed digit) [LX200 GPS Only]
     */
    virtual void handleSetReticuleDutyCycle(int n);

    // Sync Control
    /**
     * :CL#    Synchonize the telescope with the current Selenographic coordinates.
     */
    virtual void handleSynchronizeSelenographicCoordinates();

    /**
     * :CM#    Synchronizes the telescope's position with the currently selected database object's coordinates.
     */
    virtual void handleSynchronizeDatabaseCoordinates();

    // Distance Bars
    /**
     * :D#     Requests a string of bars indicating the distance to the current library object.
     */
    virtual void handleRequestDistanceBars();

    // Fan Command
    /**
     * :f+#    LX 16”– Turn on the tube exhaust fan, LX200GPS – Turn on power to accessor panel
     */
    virtual void handleTurnOnFan();

    /**
     * :f-#    LX 16”– Turn off tube exhaust fan, LX200GPS - Turn off power to accessory panel
     */
    virtual void handleTurnOffFan();

    /**
     * :fT#    LX200GPS – Return Optical Tube Assembly Temperature
     */
    virtual void handleReturnOTATemperature();

    // Focuser Control
    /**
     * :F+#    Start Focuser moving inward (toward objective)
     */
    virtual void handleStartFocuserInward();

    /**
     * :F-#    Start Focuser moving outward (away from objective)
     */
    virtual void handleStartFocuserOutward();

    /**
     * :FQ#    Halt Focuser Motion
     */
    virtual void handleHaltFocuserMotion();

    /**
     * :FF#    Set Focus speed to fastest setting
     */
    virtual void handleSetFocusSpeedFastest();

    /**
     * :FS#    Set Focus speed to slowest setting
     */
    virtual void handleSetFocusSpeedSlowest();

    /**
     * :F<n>#  Autostar & LX200GPS – set focuser speed to <n> where <n> is an ASCII digit 1..4
     */
    virtual void handleSetFocuserSpeed(int n);

    // GPS/Magnetometer commands
    /**
     * :g+#    LX200GPS Only - Turn on GPS
     */
    virtual void handleTurnOnGPS();

    /**
     * :g-#    LX200GPS Only - Turn off GPS
     */
    virtual void handleTurnOffGPS();

    /**
     * :gps#   LX200GPS Only – Turns on NMEA GPS data stream.
     */
    virtual void handleTurnOnNMEAGPSDataStream();

    /**
     * :gT#    Powers up the GPS and updates the system time from the GPS stream.
     */
    virtual void handleUpdateSystemTimeFromGPS();

    // Get Telescope Information
    /**
     * :G0#    Get Alignment Menu Entry 0 [LX200 legacy command]
     */
    virtual void handleGetAlignmentMenuEntry0();

    /**
     * :G1#    Get Alignment Menu Entry 0 [LX200 legacy command]
     */
    virtual void handleGetAlignmentMenuEntry1();

    /**
     * :G2#    Get Alignment Menu Entry 0 [LX200 legacy command]
     */
    virtual void handleGetAlignmentMenuEntry2();

    /**
     * :GA#    Get Telescope Altitude
     */
    virtual void handleGetTelescopeAltitude();

    /**
     * :Ga#    Get Local Telescope Time In 12 Hour Format
     */
    virtual void handleGetLocalTelescopeTime12Hour();

    /**
     * :Gb#    Get Browse Brighter Magnitude Limit
     */
    virtual void handleGetBrowseBrighterMagnitudeLimit();

    /**
     * :GC#    Get current date.
     */
    virtual void handleGetCurrentDate();

    /**
     * :Gc#    Get Calendar Format
     */
    virtual void handleGetCalendarFormat();

    /**
     * :GD#    Get Telescope Declination.
     */
    virtual void handleGetTelescopeDeclination();

    /**
     * :Gd#    Get Currently Selected Object/Target Declination
     */
    virtual void handleGetCurrentTargetDeclination();

    /**
     * :GF#    Get Find Field Diameter
     */
    virtual void handleGetFindFieldDiameter();

    /**
     * :Gf#    Get Browse Faint Magnitude Limit
     */
    virtual void handleGetBrowseFaintMagnitudeLimit();

    /**
     * :GG#    Get UTC offset time
     */
    virtual void handleGetUTCOffsetTime();

    /**
     * :Gg#    Get Current Site Longitude
     */
    virtual void handleGetCurrentSiteLongitude();

    /**
     * :Gh#    Get High Limit
     */
    virtual void handleGetHighLimit();

    /**
     * :GL#    Get Local Time in 24 hour format
     */
    virtual void handleGetLocalTime24Hour();

    /**
     * :Gl#    Get Larger Size Limit
     */
    virtual void handleGetLargerSizeLimit();

    /**
     * :GM#    Get Site 1 Name
     */
    virtual void handleGetSite1Name();

    /**
     * :GN#    Get Site 2 Name
     */
    virtual void handleGetSite2Name();

    /**
     * :GO#    Get Site 3 Name
     */
    virtual void handleGetSite3Name();

    /**
     * :GP#    Get Site 4 Name
     */
    virtual void handleGetSite4Name();

    /**
     * :Go#    Get Lower Limit
     */
    virtual void handleGetLowerLimit();

    /**
     * :Gq#    Get Minimum Quality For Find Operation
     */
    virtual void handleGetMinimumQualityForFind();

    /**
     * :GR#    Get Telescope RA
     */
    virtual void handleGetTelescopeRA();

    /**
     * :Gr#    Get current/target object RA
     */
    virtual void handleGetCurrentTargetRA();

    /**
     * :GS#    Get the Sidereal Time
     */
    virtual void handleGetSiderealTime();

    /**
     * :Gs#    Get Smaller Size Limit
     */
    virtual void handleGetSmallerSizeLimit();

    /**
     * :GT#    Get tracking rate
     */
    virtual void handleGetTrackingRate();

    /**
     * :Gt#    Get Current Site Latitude
     */
    virtual void handleGetCurrentSiteLatitude();

    /**
     * :GVD#   Get Telescope Firmware Date
     */
    virtual void handleGetFirmwareDate();

    /**
     * :GVN#   Get Telescope Firmware Number
     */
    virtual void handleGetFirmwareNumber();

    /**
     * :GVP#   Get Telescope Product Name
     */
    virtual void handleGetProductName();

    /**
     * :GVT#   Get Telescope Firmware Time
     */
    virtual void handleGetFirmwareTime();

    /**
     * :Gy#    Get deepsky object search string
     */
    virtual void handleGetDeepSkyObjectSearchString();

    /**
     * :GZ#    Get telescope azimuth
     */
    virtual void handleGetTelescopeAzimuth();

    // Home Position Commands
    /**
     * :hS#    LX200GPS and LX 16” Seeks Home Position and stores the encoder values
     */
    virtual void handleSeekHomePosition();

    /**
     * :hF#    LX200GPS and LX 16” Seeks the Home Position of the scope and sets/aligns
     */
    virtual void handleSeekAndAlignHomePosition();

    /**
     * :hN#    LX200GPS only: Sleep Telescope.
     */
    virtual void handleSleepTelescope();

    /**
     * :hP#    Autostar, LX200GPS and LX 16”Slew to Park Position
     */
    virtual void handleSlewToParkPosition();

    /**
     * :hW#    LX200 GPS Only: Wake up sleeping telescope.
     */
    virtual void handleWakeUpTelescope();

    /**
     * :h?#    Autostar, LX200GPS and LX 16” Query Home Status
     */
    virtual void handleQueryHomeStatus();

    // Time Format Command
    /**
     * :H#     Toggle Between 24 and 12 hour time format
     */
    virtual void handleToggleTimeFormat();

    // Initialize Telescope Command
    /**
     * :I#     LX200 GPS Only - Causes the telescope to cease current operations and restart at its power on initialization.
     */
    virtual void handleInitializeTelescope();

    // Object Library Commands
    /**
     * :LB#    Find previous object and set it as the current target object.
     */
    virtual void handleFindPreviousObject();

    /**
     * :LCNNNN# Set current target object to deep sky catalog object number NNNN
     */
    virtual void handleSetTargetObjectDeepSky(int n);

    /**
     * :LF#    Find Object using the current Size, Type, Upper limit, lower limit and Quality constraints and set it as current target object.
     */
    virtual void handleFindObjectWithConstraints();

    /**
     * :Lf#    Identify object in current field.
     */
    virtual void handleIdentifyObjectInField();

    /**
     * :LI#    Get Object Information
     */
    virtual void handleGetObjectInformation();

    /**
     * :LMNNNN# Set current target object to Messier Object NNNN, an ASCII expressed decimal number.
     */
    virtual void handleSetTargetObjectMessier(int n);

    /**
     * :LN#    Find next deep sky target object subject to the current constraints.
     */
    virtual void handleFindNextDeepSkyObject();

    /**
     * :LoD#   Select deep sky Library where D specifies
     */
    virtual void handleSelectDeepSkyLibrary(int d);

    /**
     * :LsD#   Select star catalog D, an ASCII integer where D specifies
     */
    virtual void handleSelectStarCatalog(int d);

    /**
     * :LSNNNN# Select star NNNN as the current target object from the currently selected catalog
     */
    virtual void handleSelectStar(int n);

    // Telescope Movement Commands
    /**
     * :MA#    Autostar, LX 16”, LX200GPS – Slew to target Alt and Az
     */
    virtual void handleSlewToAltAz();

    /**
     * :Me#    Move Telescope East at current slew rate
     */
    virtual void handleMoveEast();

    /**
     * :Mn#    Move Telescope North at current slew rate
     */
    virtual void handleMoveNorth();

    /**
     * :Ms#    Move Telescope South at current slew rate
     */
    virtual void handleMoveSouth();

    /**
     * :Mw#    Move Telescope West at current slew rate
     */
    virtual void handleMoveWest();

    /**
     * :MS#    Slew to Target Object
     */
    virtual void handleSlewToTargetObject();

    // High Precision Toggle
    /**
     * :P#     Toggles High Precision Pointing.
     */
    virtual void handleToggleHighPrecision();

    // Smart Drive Control
    /**
     * $Q#     Toggles Smart Drive PEC on and off for both axis
     */
    virtual void handleToggleSmartDrive();

    /**
     * :$QA+   Enable Dec/Alt PEC [LX200gps only]
     */
    virtual void handleEnableDecAltPEC();

    /**
     * :$QA-   Enable Dec/Alt PEC [LX200gps only]
     */
    virtual void handleDisableDecAltPEC();

    /**
     * :$QZ+   Enable RA/AZ PEC compensation [LX200gps only]
     */
    virtual void handleEnableRAAzPEC();

    /**
     * :$QZ-   Disable RA/AZ PEC Compensation [LX200gps only]
     */
    virtual void handleDisableRAAzPEC();

    // Movement Commands
    /**
     * :Q#     Halt all current slewing
     */
    virtual void handleHaltAllSlewing();

    /**
     * :Qe#    Halt eastward Slews
     */
    virtual void handleHaltEastwardSlews();

    /**
     * :Qn#    Halt northward Slews
     */
    virtual void handleHaltNorthwardSlews();

    /**
     * :Qs#    Halt southward Slews
     */
    virtual void handleHaltSouthwardSlews();

    /**
     * :Qw#    Halt westward Slews
     */
    virtual void handleHaltWestwardSlews();

    // Field Derotator Commands
    /**
     * :r+#    Turn on Field Derotator [LX 16” and LX200GPS]
     */
    virtual void handleTurnOnFieldDerotator();

    /**
     * :r-#    Turn off Field Derotator, halt slew in progress. [Lx 16” and LX200GPS]
     */
    virtual void handleTurnOffFieldDerotator();

    // Slew Rate Commands
    /**
     * :RC#    Set Slew rate to Centering rate (2nd slowest)
     */
    virtual void handleSetSlewRateCentering();

    /**
     * :RG#    Set Slew rate to Guiding Rate (slowest)
     */
    virtual void handleSetSlewRateGuiding();

    /**
     * :RM#    Set Slew rate to Find Rate (2nd Fastest)
     */
    virtual void handleSetSlewRateFind();

    /**
     * :RS#    Set Slew rate to max (fastest)
     */
    virtual void handleSetSlewRateMax();

    /**
     * :RADD.D# Set RA/Azimuth Slew rate to DD.D degrees per second [LX200GPS Only]
     */
    virtual void handleSetRAAzimuthSlewRate(double dd);

    /**
     * :REDD.D# Set Dec/Elevation Slew rate to DD.D degrees per second [LX200GPS only]
     */
    virtual void handleSetDecElevationSlewRate(double dd);

    /**
     * :RgSS.S# Set guide rate to +/- SS.S to arc seconds per second.
     */
    virtual void handleSetGuideRate(double ss);

    // Telescope Set Commands
    /**
     * :SasDD*MM# Set target object altitude to sDD*MM# or sDD*MM’SS# [LX 16”, Autostar, LX200GPS]
     */
    virtual void handleSetTargetAltitude(int s, int dd, int mm);

    /**
     * :SbsMM.M#  Set Brighter limit to the ASCII decimal magnitude string. SMM.M
     */
    virtual void handleSetBrighterLimit(double mm);

    /**
     * :SBn#      Set Baud Rate n, where n is an ASCII digit (1..9)
     */
    virtual void handleSetBaudRate(int n);

    /**
     * :SCMM/DD/YY# Change Handbox Date to MM/DD/YY
     */
    virtual void handleChangeHandboxDate(int mm, int dd, int yy);

    /**
     * :SdsDD*MM# Set target object declination to sDD*MM
     */
    virtual void handleSetTargetDeclination(int s, int dd, int mm);
};

#endif
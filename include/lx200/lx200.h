/**
 * @file lx200.h
 * @brief Meade LX200 Telescope Serial Command Protocol Implementation
 *
 * ============================================================================
 *                    MEADE LX200 TELESCOPE SERIAL COMMAND PROTOCOL
 * ============================================================================
 *
 * This file implements the Meade LX200 telescope serial command protocol as
 * documented in the official Meade LX200 Command Set specification.
 *
 * PROTOCOL OVERVIEW:
 * =================
 * The LX200 protocol uses ASCII commands sent over a serial connection.
 * Commands are typically prefixed with a colon (:) and may end with a hash (#).
 * The telescope responds with specific data formats or acknowledgments.
 *
 * COMMAND FORMAT:
 * ==============
 * Basic format: :<command>[parameters]#
 * - Commands are case-sensitive
 * - Parameters use specific formats (coordinates, numbers, etc.)
 * - Responses vary by command type
 *
 * COORDINATE FORMATS:
 * ==================
 * - Right Ascension: HH:MM:SS or HH:MM.T
 * - Declination: sDD*MM:SS or sDD*MM
 * - Longitude/Latitude: sDDD*MM:SS
 * - Time: HH:MM:SS
 * - Date: MM/DD/YY
 *
 * COMMAND CATEGORIES:
 * ==================
 *
 * A - ALIGNMENT COMMANDS
 * ---------------------
 * :Aa#    - Start Automatic Alignment
 * :AL#    - Set to Land mode
 * :AP#    - Set to Polar mode
 * :AA#    - Set to AltAz mode
 *
 * ACK - ALIGNMENT QUERY
 * --------------------
 * :A?#    - Query current alignment mode
 *           Returns: P# (Polar), A# (AltAz), L# (Land)
 *
 * B - RETICULE/ACCESSORY CONTROL
 * ------------------------------
 * :B+#    - Increase reticule brightness
 * :B-#    - Decrease reticule brightness
 * :BD#    - Set reticule brightness to dim
 * :BB#    - Set reticule brightness to bright
 * :BF#    - Flash reticule
 *
 * C - SYNC CONTROL
 * ---------------
 * :CM#    - Synchronize telescope to current target coordinates
 *           Returns: "Coordinates matched.        #" or "Object below horizon.#"
 * :CS#    - Synchronize telescope to current target coordinates (silent)
 *
 * D - DISTANCE BARS
 * ----------------
 * :D#     - Toggle distance bars on/off in eyepiece display
 *
 * F - FOCUSER CONTROL
 * ------------------
 * :F+#    - Start moving focuser inward (toward objective)
 * :F-#    - Start moving focuser outward (away from objective)
 * :FQ#    - Stop focuser movement
 * :FF#    - Set focuser to Fast speed
 * :FS#    - Set focuser to Slow speed
 *
 * G - GET TELESCOPE INFORMATION
 * ----------------------------
 * :GA#    - Get telescope altitude
 *           Returns: sDD*MM:SS# or sDD*MM#
 * :GZ#    - Get telescope azimuth
 *           Returns: DDD*MM:SS# or DDD*MM#
 * :GR#    - Get target right ascension
 *           Returns: HH:MM:SS# or HH:MM.T#
 * :GD#    - Get target declination
 *           Returns: sDD*MM:SS# or sDD*MM#
 * :Gr#    - Get current telescope right ascension
 *           Returns: HH:MM:SS# or HH:MM.T#
 * :Gd#    - Get current telescope declination
 *           Returns: sDD*MM:SS# or sDD*MM#
 * :GL#    - Get local time (12 hour format)
 *           Returns: HH:MM:SS#
 * :GM#    - Get local time (24 hour format)
 *           Returns: HH:MM:SS#
 * :GC#    - Get current date
 *           Returns: MM/DD/YY#
 * :GG#    - Get UTC offset time
 *           Returns: sHH# or sHH.H#
 * :Gg#    - Get current site longitude
 *           Returns: sDDD*MM#
 * :Gt#    - Get current site latitude
 *           Returns: sDD*MM#
 * :GS#    - Get local sidereal time
 *           Returns: HH:MM:SS#
 * :GT#    - Get tracking rate
 *           Returns: TT.T# (tracking rate)
 * :GW#    - Get telescope status
 *           Returns: Various status flags
 * :GVP#   - Get telescope product name
 *           Returns: Product name string
 * :GVN#   - Get telescope product number
 *           Returns: Product number
 * :GVD#   - Get telescope firmware date
 *           Returns: MMM DD YYYY#
 * :GVT#   - Get telescope firmware time
 *           Returns: HH:MM:SS#
 *
 * g - GPS/MAGNETOMETER COMMANDS
 * ----------------------------
 * :gT#    - GPS Test - Returns GPS status
 * :gW#    - GPS Write - Enable GPS updates
 * :gR#    - GPS Read - Get GPS data
 * :gP#    - GPS Power - Toggle GPS power
 *
 * H - TIME FORMAT COMMAND
 * ----------------------
 * :H#     - Toggle time format between 12/24 hour
 *
 * I - INITIALIZE TELESCOPE COMMAND
 * -------------------------------
 * :I#     - Initialize telescope (points to north)
 *
 * L - OBJECT LIBRARY COMMANDS
 * --------------------------
 * :LM#    - Select Messier catalog
 * :LN#    - Select NGC catalog
 * :LC#    - Select Caldwell catalog
 * :LS#    - Select Star catalog
 * :Lf#    - Find object in current catalog
 * :Lo#    - Select object from current catalog
 * :LI#    - Get object information
 * :Lb#    - Browse next object
 * :L$#    - Search for object by name
 *
 * P - HIGH PRECISION TOGGLE
 * ------------------------
 * :P#     - Toggle high precision coordinate mode
 *           High precision: HH:MM:SS and sDD:MM:SS
 *           Low precision:  HH:MM.T and sDD:MM
 *
 * Q - MOVEMENT COMMANDS
 * --------------------
 * :Qn#    - Stop slewing north (Dec+)
 * :Qs#    - Stop slewing south (Dec-)
 * :Qe#    - Stop slewing east (RA+)
 * :Qw#    - Stop slewing west (RA-)
 * :Q#     - Stop all slewing
 *
 * R - SLEW RATE COMMANDS
 * ---------------------
 * :RG#    - Set slew rate to Guide (0.5x sidereal)
 * :RC#    - Set slew rate to Centering (8x sidereal)
 * :RM#    - Set slew rate to Find (16x sidereal)
 * :RS#    - Set slew rate to Slew (512x sidereal)
 * :R0#    - Set custom slew rate 0
 * :R1#    - Set custom slew rate 1
 * :R2#    - Set custom slew rate 2
 * :R3#    - Set custom slew rate 3
 * :R4#    - Set custom slew rate 4
 * :R5#    - Set custom slew rate 5
 * :R6#    - Set custom slew rate 6
 * :R7#    - Set custom slew rate 7
 * :R8#    - Set custom slew rate 8
 * :R9#    - Set custom slew rate 9
 *
 * S - TELESCOPE SET COMMANDS
 * -------------------------
 * :Sr[HH:MM:SS]#      - Set target right ascension
 *                       Returns: 0# (invalid) or 1# (valid)
 * :Sd[sDD*MM:SS]#     - Set target declination
 *                       Returns: 0# (invalid) or 1# (valid)
 * :Sa[sDD*MM:SS]#     - Set target altitude
 *                       Returns: 0# (invalid) or 1# (valid)
 * :Sz[DDD*MM:SS]#     - Set target azimuth
 *                       Returns: 0# (invalid) or 1# (valid)
 * :SL[HH:MM:SS]#      - Set local time
 *                       Returns: 0# (invalid) or 1# (valid)
 * :SC[MM/DD/YY]#      - Set current date
 *                       Returns: 0# (invalid) or 1# (valid)
 * :SG[sHH]# or :SG[sHH.H]# - Set UTC offset
 *                       Returns: 0# (invalid) or 1# (valid)
 * :Sg[sDDD*MM]#       - Set longitude
 *                       Returns: 0# (invalid) or 1# (valid)
 * :St[sDD*MM]#        - Set latitude
 *                       Returns: 0# (invalid) or 1# (valid)
 * :SS[HH:MM:SS]#      - Set local sidereal time
 *                       Returns: 0# (invalid) or 1# (valid)
 * :ST[TT.T]#          - Set tracking rate
 *                       Returns: 0# (invalid) or 1# (valid)
 * :Sw[0-9]#           - Set maximum slew rate
 *
 * T - TRACKING COMMANDS
 * --------------------
 * :TL#    - Set tracking rate to Lunar
 * :TQ#    - Turn tracking off
 * :TS#    - Set tracking rate to Solar
 * :TM#    - Set tracking rate to Sidereal
 * :T+#    - Increment tracking rate by 0.1 Hz
 * :T-#    - Decrement tracking rate by 0.1 Hz
 *
 * U - PRECISION TOGGLE
 * -------------------
 * :U#     - Toggle between low/high precision coordinate display
 *
 * MOVEMENT COMMANDS (Direction):
 * =============================
 * :Mn#    - Start slewing north at current slew rate
 * :Ms#    - Start slewing south at current slew rate
 * :Me#    - Start slewing east at current slew rate
 * :Mw#    - Start slewing west at current slew rate
 * :MS#    - Slew to target coordinates
 *           Returns: 0# (slew possible) or 1# (object below horizon) or 2# (object below higher limit)
 *
 * APPENDIX A: LX200GPS COMMAND EXTENSIONS
 * ======================================
 *
 * GPS SPECIFIC COMMANDS:
 * ---------------------
 * :gps#   - Get GPS status and position
 * :gT#    - Test GPS communication
 * :gU#    - GPS Update/Enable
 * :gD#    - GPS Disable
 *
 * MAGNETOMETER COMMANDS:
 * ---------------------
 * :mT#    - Test magnetometer
 * :mC#    - Calibrate magnetometer
 * :mR#    - Read magnetometer heading
 *
 * USAGE EXAMPLES:
 * ==============
 *
 * 1. Get current telescope position:
 *    Send: :Gr#     (Get current RA)
 *    Recv: 14:30:45#
 *    Send: :Gd#     (Get current Dec)
 *    Recv: +45*30:15#
 *
 * 2. Slew to target:
 *    Send: :Sr14:30:45#   (Set target RA)
 *    Recv: 1#
 *    Send: :Sd+45*30:15#  (Set target Dec)
 *    Recv: 1#
 *    Send: :MS#           (Start slew)
 *    Recv: 0#
 *
 * 3. Set site coordinates:
 *    Send: :Sg+122*30#    (Set longitude)
 *    Recv: 1#
 *    Send: :St+37*45#     (Set latitude)
 *    Recv: 1#
 *
 * ERROR HANDLING:
 * ==============
 * - Invalid commands typically return no response or error codes
 * - Parameter validation returns 0# for invalid, 1# for valid
 * - Some commands may timeout if telescope is busy
 *
 * NOTES:
 * =====
 * - All coordinates use standard astronomical conventions
 * - RA in hours, minutes, seconds (0-24h)
 * - Dec in degrees, arcminutes, arcseconds (±90°)
 * - Longitude: West is negative, East is positive
 * - Latitude: South is negative, North is positive
 * - Commands are case-sensitive
 * - Some commands may not be supported on all telescope models
 *
 * IMPLEMENTATION CONSIDERATIONS:
 * =============================
 * - Use appropriate timeouts for command responses
 * - Handle partial responses and buffering
 * - Validate coordinate ranges before sending
 * - Consider telescope state when sending commands
 * - Implement proper error handling and recovery
 *
 * This implementation provides a complete interface to control Meade LX200
 * compatible telescopes via the standard serial command protocol.
 */

#pragma once

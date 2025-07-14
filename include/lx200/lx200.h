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
 * Basic format: :\<command\>[parameters]#
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
 *           Returns: 0# (slew possible) or 1# (object below horizon) or 2# (object below higher
 * limit)
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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup lx200_parser LX200 Protocol Parser
 * @brief Parser and handler functions for the Meade LX200 telescope protocol
 * @{
 */

/* ============================================================================
 * CONSTANTS AND DEFINITIONS
 * ============================================================================ */

/** Maximum length of an LX200 command including terminator */
#define LX200_MAX_COMMAND_LENGTH 32

/** Maximum length of an LX200 response including terminator */
#define LX200_MAX_RESPONSE_LENGTH 64

/** LX200 command prefix character */
#define LX200_COMMAND_PREFIX ':'

/** LX200 command terminator character */
#define LX200_COMMAND_TERMINATOR '#'

/** LX200 response terminator character */
#define LX200_RESPONSE_TERMINATOR '#'

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/**
 * @brief LX200 command parsing result codes
 */
typedef enum {
	/** Command parsed successfully */
	LX200_PARSE_OK = 0,
	/** Command is incomplete, need more data */
	LX200_PARSE_INCOMPLETE,
	/** Invalid command prefix */
	LX200_PARSE_INVALID_PREFIX,
	/** Invalid or missing terminator */
	LX200_PARSE_INVALID_TERMINATOR,
	/** Unknown or malformed command */
	LX200_PARSE_INVALID_COMMAND,
	/** Invalid parameter format */
	LX200_PARSE_INVALID_PARAMETER,
	/** Command too long for buffer */
	LX200_PARSE_BUFFER_OVERFLOW,
	/** General parsing error */
	LX200_PARSE_ERROR
} lx200_parse_result_t;

/**
 * @brief LX200 command categories
 */
typedef enum {
	/** Alignment commands (A) */
	LX200_CMD_ALIGNMENT,
	/** Reticle/accessory control (B) */
	LX200_CMD_RETICLE,
	/** Sync control (C) */
	LX200_CMD_SYNC,
	/** Distance bars (D) */
	LX200_CMD_DISTANCE,
	/** Focuser control (F) */
	LX200_CMD_FOCUSER,
	/** Get telescope information (G) */
	LX200_CMD_GET,
	/** GPS/magnetometer commands (g) */
	LX200_CMD_GPS,
	/** Time format command (H) */
	LX200_CMD_TIME_FORMAT,
	/** Initialize telescope (I) */
	LX200_CMD_INITIALIZE,
	/** Object library commands (L) */
	LX200_CMD_LIBRARY,
	/** Movement commands (M) */
	LX200_CMD_MOVE,
	/** High precision toggle (P) */
	LX200_CMD_PRECISION,
	/** Stop movement commands (Q) */
	LX200_CMD_STOP,
	/** Slew rate commands (R) */
	LX200_CMD_SLEW_RATE,
	/** Set telescope parameters (S) */
	LX200_CMD_SET,
	/** Tracking commands (T) */
	LX200_CMD_TRACKING,
	/** Precision toggle (U) */
	LX200_CMD_PRECISION_TOGGLE,
	/** Unknown command category */
	LX200_CMD_UNKNOWN
} lx200_command_category_t;

/**
 * @brief LX200 coordinate formats
 */
typedef enum {
	/** HH:MM.T, sDD*MM */
	LX200_COORD_LOW_PRECISION,
	/** HH:MM:SS, sDD*MM:SS */
	LX200_COORD_HIGH_PRECISION
} lx200_precision_t;

/**
 * @brief LX200 alignment modes
 */
typedef enum {
	/** Polar alignment mode */
	LX200_ALIGN_POLAR,
	/** Alt-Az alignment mode */
	LX200_ALIGN_ALTAZ,
	/** Land alignment mode */
	LX200_ALIGN_LAND
} lx200_alignment_mode_t;

/**
 * @brief LX200 tracking rates
 */
typedef enum {
	/** Tracking disabled */
	LX200_TRACK_OFF,
	/** Sidereal tracking rate */
	LX200_TRACK_SIDEREAL,
	/** Solar tracking rate */
	LX200_TRACK_SOLAR,
	/** Lunar tracking rate */
	LX200_TRACK_LUNAR
} lx200_tracking_rate_t;

/**
 * @brief LX200 slew rates
 */
typedef enum {
	/** Guide rate (0.5x sidereal) */
	LX200_SLEW_GUIDE,
	/** Centering rate (8x sidereal) */
	LX200_SLEW_CENTERING,
	/** Find rate (16x sidereal) */
	LX200_SLEW_FIND,
	/** Slew rate (512x sidereal) */
	LX200_SLEW_SLEW,
	/** Custom rate 0 */
	LX200_SLEW_CUSTOM_0,
	/** Custom rate 1 */
	LX200_SLEW_CUSTOM_1,
	/** Custom rate 2 */
	LX200_SLEW_CUSTOM_2,
	/** Custom rate 3 */
	LX200_SLEW_CUSTOM_3,
	/** Custom rate 4 */
	LX200_SLEW_CUSTOM_4,
	/** Custom rate 5 */
	LX200_SLEW_CUSTOM_5,
	/** Custom rate 6 */
	LX200_SLEW_CUSTOM_6,
	/** Custom rate 7 */
	LX200_SLEW_CUSTOM_7,
	/** Custom rate 8 */
	LX200_SLEW_CUSTOM_8,
	/** Custom rate 9 */
	LX200_SLEW_CUSTOM_9
} lx200_slew_rate_t;

/* ============================================================================
 * STRUCTURE DEFINITIONS
 * ============================================================================ */

/**
 * @brief LX200 coordinate structure
 */
typedef struct {
	/** Degrees component */
	int16_t degrees;
	/** Minutes component */
	uint8_t minutes;
	/** Seconds component */
	uint8_t seconds;
	/** Tenths of minutes (low precision mode) */
	uint8_t tenths;
	/** True if coordinate is negative */
	bool is_negative;
	/** Coordinate precision */
	lx200_precision_t precision;
} lx200_coordinate_t;

/**
 * @brief LX200 time structure
 */
typedef struct {
	/** Hours (0-23) */
	uint8_t hours;
	/** Minutes (0-59) */
	uint8_t minutes;
	/** Seconds (0-59) */
	uint8_t seconds;
	/** True for 24h format, false for 12h */
	bool is_24h_format;
} lx200_time_t;

/**
 * @brief LX200 date structure
 */
typedef struct {
	/** Month (1-12) */
	uint8_t month;
	/** Day (1-31) */
	uint8_t day;
	/** Year (0-99, represents 2000-2099) */
	uint8_t year;
} lx200_date_t;

/**
 * @brief LX200 parsed command structure
 */
typedef struct {
	/** Command category */
	lx200_command_category_t category;
	/** Command string (up to 3 chars + null) */
	char command[4];
	/** Command parameter */
	char parameter[LX200_MAX_COMMAND_LENGTH];
	/** Length of parameter */
	size_t parameter_length;
	/** True if command has parameter */
	bool has_parameter;
} lx200_command_t;

/**
 * @brief LX200 parser state structure
 */
typedef struct {
	/** Input buffer */
	char buffer[LX200_MAX_COMMAND_LENGTH];
	/** Current buffer length */
	size_t buffer_length;
	/** True when command is complete */
	bool command_complete;
	/** Current precision mode */
	lx200_precision_t precision_mode;
} lx200_parser_state_t;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Initialize LX200 parser state
 * @param state Pointer to parser state structure
 */
void lx200_parser_init(lx200_parser_state_t *state);

/**
 * @brief Reset LX200 parser state
 * @param state Pointer to parser state structure
 */
void lx200_parser_reset(lx200_parser_state_t *state);

/**
 * @brief Add data to LX200 parser buffer
 * @param state Pointer to parser state structure
 * @param data Pointer to input data
 * @param length Length of input data
 * @return Parse result code
 */
lx200_parse_result_t lx200_parser_add_data(lx200_parser_state_t *state, const char *data,
					   size_t length);

/**
 * @brief Parse complete LX200 command
 * @param state Pointer to parser state structure
 * @param command Pointer to output command structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_command(const lx200_parser_state_t *state,
					 lx200_command_t *command);

/**
 * @brief Parse LX200 command from string
 * @param cmd_string Command string to parse
 * @param command Pointer to output command structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_command_string(const char *cmd_string, lx200_command_t *command);

/* ============================================================================
 * COORDINATE PARSING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Parse right ascension coordinate
 * @param str Input string (HH:MM:SS or HH:MM.T format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_ra_coordinate(const char *str, lx200_coordinate_t *coord);

/**
 * @brief Parse declination coordinate
 * @param str Input string (sDD*MM:SS or sDD*MM format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_dec_coordinate(const char *str, lx200_coordinate_t *coord);

/**
 * @brief Parse altitude coordinate
 * @param str Input string (sDD*MM:SS or sDD*MM format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_alt_coordinate(const char *str, lx200_coordinate_t *coord);

/**
 * @brief Parse azimuth coordinate
 * @param str Input string (DDD*MM:SS or DDD*MM format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_az_coordinate(const char *str, lx200_coordinate_t *coord);

/**
 * @brief Parse longitude coordinate
 * @param str Input string (sDDD*MM format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_longitude(const char *str, lx200_coordinate_t *coord);

/**
 * @brief Parse latitude coordinate
 * @param str Input string (sDD*MM format)
 * @param coord Pointer to output coordinate structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_latitude(const char *str, lx200_coordinate_t *coord);

/* ============================================================================
 * TIME AND DATE PARSING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Parse time value
 * @param str Input string (HH:MM:SS format)
 * @param time Pointer to output time structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_time(const char *str, lx200_time_t *time);

/**
 * @brief Parse date value
 * @param str Input string (MM/DD/YY format)
 * @param date Pointer to output date structure
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_date(const char *str, lx200_date_t *date);

/**
 * @brief Parse UTC offset
 * @param str Input string (sHH or sHH.H format)
 * @param offset Pointer to output offset in hours
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_utc_offset(const char *str, float *offset);

/* ============================================================================
 * PARAMETER PARSING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Parse tracking rate parameter
 * @param str Input string (TT.T format)
 * @param rate Pointer to output tracking rate
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_tracking_rate(const char *str, float *rate);

/**
 * @brief Parse slew rate parameter
 * @param str Input string (single digit 0-9)
 * @param rate Pointer to output slew rate
 * @return Parse result code
 */
lx200_parse_result_t lx200_parse_slew_rate(const char *str, lx200_slew_rate_t *rate);

/* ============================================================================
 * FORMATTING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Format right ascension coordinate to string
 * @param coord Pointer to coordinate structure
 * @param str Output string buffer
 * @param str_size Size of output buffer
 * @return Number of characters written, or negative on error
 */
int lx200_format_ra_coordinate(const lx200_coordinate_t *coord, char *str, size_t str_size);

/**
 * @brief Format declination coordinate to string
 * @param coord Pointer to coordinate structure
 * @param str Output string buffer
 * @param str_size Size of output buffer
 * @return Number of characters written, or negative on error
 */
int lx200_format_dec_coordinate(const lx200_coordinate_t *coord, char *str, size_t str_size);

/**
 * @brief Format time to string
 * @param time Pointer to time structure
 * @param str Output string buffer
 * @param str_size Size of output buffer
 * @return Number of characters written, or negative on error
 */
int lx200_format_time(const lx200_time_t *time, char *str, size_t str_size);

/**
 * @brief Format date to string
 * @param date Pointer to date structure
 * @param str Output string buffer
 * @param str_size Size of output buffer
 * @return Number of characters written, or negative on error
 */
int lx200_format_date(const lx200_date_t *date, char *str, size_t str_size);

/* ============================================================================
 * VALIDATION FUNCTIONS
 * ============================================================================ */

/**
 * @brief Validate coordinate values
 * @param coord Pointer to coordinate structure
 * @param coord_type Type of coordinate (RA, Dec, Alt, Az, etc.)
 * @return true if coordinate is valid, false otherwise
 */
bool lx200_validate_coordinate(const lx200_coordinate_t *coord,
			       lx200_command_category_t coord_type);

/**
 * @brief Validate time values
 * @param time Pointer to time structure
 * @return true if time is valid, false otherwise
 */
bool lx200_validate_time(const lx200_time_t *time);

/**
 * @brief Validate date values
 * @param date Pointer to date structure
 * @return true if date is valid, false otherwise
 */
bool lx200_validate_date(const lx200_date_t *date);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get command category from command string
 * @param command Command string
 * @return Command category
 */
lx200_command_category_t lx200_get_command_category(const char *command);

/**
 * @brief Check if command expects a parameter
 * @param command Command string
 * @return true if command expects parameter, false otherwise
 */
bool lx200_command_has_parameter(const char *command);

/**
 * @brief Get expected parameter format for command
 * @param command Command string
 * @return Pointer to format description string
 */
const char *lx200_get_parameter_format(const char *command);

/**
 * @brief Convert parse result to string
 * @param result Parse result code
 * @return Pointer to result description string
 */
const char *lx200_parse_result_to_string(lx200_parse_result_t result);

/**
 * @brief Set precision mode
 * @param state Pointer to parser state structure
 * @param precision Precision mode to set
 */
void lx200_set_precision_mode(lx200_parser_state_t *state, lx200_precision_t precision);

/**
 * @brief Get current precision mode
 * @param state Pointer to parser state structure
 * @return Current precision mode
 */
lx200_precision_t lx200_get_precision_mode(const lx200_parser_state_t *state);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

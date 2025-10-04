/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file lx200.hpp
 * @brief LX200 Telescope Control Protocol - C++20 Implementation
 * 
 * This header provides the C++20 implementation of the Meade LX200
 * telescope control protocol parser. It replaces the previous C
 * implementation with modern C++ while maintaining compatibility
 * with ASCOM, INDI, N.I.N.A., and SkySafari software.
 * 
 * Key features:
 * - Zero dynamic allocation (embedded-safe)
 * - Incremental command parsing
 * - Type-safe coordinate handling
 * - Compile-time validation where possible
 * - <10ms parsing performance
 * 
 * @see LX200 Protocol: https://www.meade.com/support/LX200CommandSet.pdf
 */

#ifndef LX200_HPP
#define LX200_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace lx200 {

/* ========================================================================
 * Core Enumerations
 * ======================================================================== */

/**
 * @brief LX200 command family classification
 * 
 * Commands are grouped by their first character after the ':' prefix.
 * This enables fast lookup and categorization.
 */
enum class CommandFamily : uint8_t {
    Alignment    = 'A',  ///< Telescope alignment commands
    Backup       = 'B',  ///< Backlash and reticle control
    DateTime     = 'C',  ///< Date/time commands (also some G, S, L)
    Distance     = 'D',  ///< Distance bars
    Focus        = 'F',  ///< Focuser control
    GetInfo      = 'G',  ///< Get telescope information
    GPS          = 'g',  ///< GPS-related commands (lowercase g)
    Home         = 'h',  ///< Home position and parking
    Initialize   = 'I',  ///< Initialize telescope
    Library      = 'L',  ///< Object library commands
    Movement     = 'M',  ///< Slew and movement control
    Precision    = 'P',  ///< Toggle precision mode
    Quit         = 'Q',  ///< Stop/quit movement
    Rate         = 'R',  ///< Slew rate control
    SetInfo      = 'S',  ///< Set telescope information
    Tracking     = 'T',  ///< Tracking rate control
    User         = 'U',  ///< User-defined commands
    Extended     = 'X',  ///< Extended OAT-specific commands
    Unknown      = 0     ///< Unrecognized command
};

/**
 * @brief Coordinate precision mode
 * 
 * LX200 protocol supports two precision modes for coordinates:
 * - High: HH:MM:SS (arcsecond precision)
 * - Low: HH:MM.T (0.1 arcminute precision)
 */
enum class PrecisionMode : uint8_t {
    High,  ///< High precision (HH:MM:SS format)
    Low    ///< Low precision (HH:MM.T format)
};

/**
 * @brief Parse result codes
 * 
 * Indicates success or specific failure mode for parsing operations.
 * Success = 0 for zassert_ok compatibility
 */
enum class ParseResult : uint8_t {
    Success = 0,         ///< Parsing succeeded (0 for compatibility)
    Incomplete,          ///< Need more characters
    ErrorInvalidFormat,  ///< Format doesn't match expected pattern
    ErrorOutOfRange,     ///< Numeric value out of valid range
    ErrorBufferFull,     ///< Command exceeds buffer capacity
    ErrorGeneral         ///< Other parsing error
};

/* ========================================================================
 * Coordinate Structures
 * ======================================================================== */

/**
 * @brief Right Ascension coordinate
 * 
 * Represents celestial longitude (0h to 24h).
 * Format: HH:MM:SS (high) or HH:MM.T (low precision)
 * 
 * Note: In low precision mode, tenths of arcminutes are converted to seconds.
 * Conversion: 0.1 arcminute = 6 arcseconds (1 arcmin = 60 arcsec)
 * 
 * Low precision conversion table:
 *   .0 → 0 sec    .1 → 6 sec    .2 → 12 sec   .3 → 18 sec   .4 → 24 sec
 *   .5 → 30 sec   .6 → 36 sec   .7 → 42 sec   .8 → 48 sec   .9 → 54 sec
 */
struct RACoordinate {
    uint8_t hours;    ///< 0-23 hours
    uint8_t minutes;  ///< 0-59 minutes
    uint8_t seconds;  ///< 0-59 arcseconds (HH:MM:SS in high precision, or tenths*6 in low precision)
    
    /// Compile-time validation
    static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) noexcept {
        return h < 24 && m < 60 && s < 60;
    }
};

/**
 * @brief Declination coordinate
 * 
 * Represents celestial latitude (-90° to +90°).
 * Format: sDD*MM:SS or sDD*MM'SS
 */
struct DECCoordinate {
    char sign;           ///< '+' or '-'
    uint8_t degrees;     ///< 0-90 degrees
    uint8_t arcminutes;  ///< 0-59 arcminutes
    uint8_t arcseconds;  ///< 0-59 arcseconds (or 0 in low precision)
    
    /// Compile-time validation
    static constexpr bool is_valid(char s, uint8_t d, uint8_t m, uint8_t a) noexcept {
        return (s == '+' || s == '-') && d <= 90 && m < 60 && a < 60;
    }
    
    /// Convert to decimal degrees
    constexpr double to_degrees() const noexcept {
        double value = degrees + arcminutes/60.0 + arcseconds/3600.0;
        return (sign == '-') ? -value : value;
    }
};

/**
 * @brief Geographic latitude
 * 
 * Represents site latitude (-90° to +90°).
 * Format: sDD*MM
 */
struct LatitudeCoordinate {
    char sign;           ///< '+' (N) or '-' (S)
    uint8_t degrees;     ///< 0-90 degrees
    uint8_t arcminutes;  ///< 0-59 arcminutes
    
    static constexpr bool is_valid(char s, uint8_t d, uint8_t m) noexcept {
        return (s == '+' || s == '-') && d <= 90 && m < 60;
    }
};

/**
 * @brief Geographic longitude
 * 
 * Represents site longitude (0° to 360° west from Greenwich).
 * Format: DDD*MM
 */
struct LongitudeCoordinate {
    uint16_t degrees;    ///< 0-359 degrees
    uint8_t arcminutes;  ///< 0-59 arcminutes
    
    static constexpr bool is_valid(uint16_t d, uint8_t m) noexcept {
        return d < 360 && m < 60;
    }
};

/**
 * @brief Time value
 * 
 * Represents local or UTC time.
 * Format: HH:MM:SS
 */
struct TimeValue {
    uint8_t hours;    ///< 0-23 hours
    uint8_t minutes;  ///< 0-59 minutes
    uint8_t seconds;  ///< 0-59 seconds
    
    static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) noexcept {
        return h < 24 && m < 60 && s < 60;
    }
};

/**
 * @brief Date value
 * 
 * Represents calendar date.
 * Format: MM/DD/YY (year 2000-2099)
 */
struct DateValue {
    uint8_t month;  ///< 1-12
    uint8_t day;    ///< 1-31
    uint8_t year;   ///< 0-99 (interpreted as 2000-2099)
    
    static constexpr bool is_valid(uint8_t m, uint8_t d, uint8_t y) noexcept {
        return m >= 1 && m <= 12 && d >= 1 && d <= 31 && y <= 99;
    }
};

/* ========================================================================
 * Command Structure
 * ======================================================================== */

/**
 * @brief Parsed LX200 command
 * 
 * Represents a complete command with family classification and
 * optional parameter string. Uses string_view for non-owning
 * references to the parser's buffer.
 */
struct Command {
    CommandFamily family;         ///< Command family classification
    std::string_view name;        ///< Full command name (e.g., "GR", "Sr")
    std::string_view parameters;  ///< Parameter substring (empty if none)
    
    /// Check if command has parameters
    constexpr bool has_parameters() const noexcept {
        return !parameters.empty();
    }
    
    /// Get first character of command (after ':')
    constexpr char first_char() const noexcept {
        return name.empty() ? '\0' : name[0];
    }
};

/* ========================================================================
 * Parser State
 * ======================================================================== */

/**
 * @brief LX200 command parser with incremental parsing
 * 
 * Maintains state for parsing LX200 commands character-by-character.
 * This enables use with UART interrupts and async I/O.
 * 
 * Usage:
 * @code
 *   ParserState parser;
 *   
 *   // Feed characters from serial port
 *   while (uart_has_data()) {
 *       char c = uart_read_char();
 *       if (parser.feed_character(c) == ParseResult::Success) {
 *           // Command is complete
 *           if (auto cmd = parser.get_command()) {
 *               process_command(*cmd);
 *           }
 *       }
 *   }
 * @endcode
 */
class ParserState {
public:
    /**
     * @brief Construct parser in initial state
     * 
     * Default precision is High mode.
     */
    ParserState() noexcept;
    
    /**
     * @brief Reset parser state to initial conditions
     * 
     * Clears buffer and command complete flag.
     * Precision mode is preserved.
     */
    void reset() noexcept;
    
    /**
     * @brief Feed one character to the parser
     * 
     * @param c Character to process
     * @return ParseResult indicating current state
     * 
     * Returns Success when '#' terminator received.
     * Returns Incomplete while building command.
     * Returns error codes for invalid input.
     */
    ParseResult feed_character(char c) noexcept;
    
    /**
     * @brief Check if a complete command is ready
     * 
     * @return true if get_command() will return a valid Command
     */
    bool is_command_ready() const noexcept;
    
    /**
     * @brief Get the parsed command (if complete)
     * 
     * @return Command object if ready, std::nullopt otherwise
     * 
     * After calling, parser resets to accept next command.
     */
    std::optional<Command> get_command() noexcept;
    
    /**
     * @brief Get current precision mode
     */
    PrecisionMode get_precision() const noexcept { return precision_; }
    
    /**
     * @brief Set precision mode
     * 
     * @param mode New precision mode
     */
    void set_precision(PrecisionMode mode) noexcept { precision_ = mode; }
    
private:
    static constexpr size_t MAX_COMMAND_LENGTH = 64;  ///< Maximum command buffer size
    
    std::array<char, MAX_COMMAND_LENGTH> buffer_;  ///< Command buffer
    size_t buffer_length_{0};                      ///< Current buffer length
    bool command_complete_{false};                 ///< Command complete flag
    PrecisionMode precision_{PrecisionMode::High}; ///< Current precision mode
    
    /// Identify command family from first character
    CommandFamily identify_family(char first_char) const noexcept;
    
    /// Split command name from parameters
    void parse_command_parts(std::string_view& name, std::string_view& params) const noexcept;
};

/* ========================================================================
 * Parsing Functions
 * ======================================================================== */

/**
 * @brief Parse Right Ascension coordinate string
 * 
 * @param str String in format HH:MM:SS or HH:MM.T
 * @param mode Precision mode (High or Low)
 * @param[out] coord Parsed coordinate on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_ra_coordinate(
    std::string_view str,
    PrecisionMode mode,
    RACoordinate& coord
) noexcept;

/**
 * @brief Parse Declination coordinate string
 * 
 * @param str String in format sDD*MM:SS or sDD*MM
 * @param mode Precision mode (High or Low)
 * @param[out] coord Parsed coordinate on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_dec_coordinate(
    std::string_view str,
    PrecisionMode mode,
    DECCoordinate& coord
) noexcept;

/**
 * @brief Parse Latitude coordinate string
 * 
 * @param str String in format sDD*MM
 * @param[out] coord Parsed coordinate on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_latitude_coordinate(
    std::string_view str,
    LatitudeCoordinate& coord
) noexcept;

/**
 * @brief Parse Longitude coordinate string
 * 
 * @param str String in format DDD*MM
 * @param[out] coord Parsed coordinate on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_longitude_coordinate(
    std::string_view str,
    LongitudeCoordinate& coord
) noexcept;

/**
 * @brief Parse Time value string
 * 
 * @param str String in format HH:MM:SS
 * @param[out] time Parsed time on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_time_value(
    std::string_view str,
    TimeValue& time
) noexcept;

/**
 * @brief Parse Date value string
 * 
 * @param str String in format MM/DD/YY
 * @param[out] date Parsed date on success
 * @return ParseResult::Success or error code
 */
ParseResult parse_date_value(
    std::string_view str,
    DateValue& date
) noexcept;

} // namespace lx200

#endif // LX200_HPP

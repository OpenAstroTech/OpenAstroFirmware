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
#include <type_traits>
#include <utility>
#include <variant>

namespace lx200
{

/* ========================================================================
 * Core Enumerations
 * ======================================================================== */

/**
 * @brief LX200 command family classification
 *
 * Commands are grouped by their first character(s) after the ':' prefix.
 * This enables fast lookup and categorization.
 * 
 * Special cases:
 * - Most commands: Single character designator (e.g., :Gxx#, :Mxx#)
 * - Dollar commands: Two-character designator starting with '$' (e.g., :$Bxx#, :$Qxx#)
 * 
 * Note: Some command families have semantic groupings across multiple
 * prefixes (e.g., date/time commands span C, G, H, S families).
 */
enum class CommandFamily : uint8_t {
	Alignment = 'A',  ///< Telescope alignment commands
	Backlash = '$',   ///< Active backlash compensation (:$B commands, LX200GPS)
	Reticle = 'B',    ///< Reticle brightness and accessory control
	Sync = 'C',       ///< Sync control (telescope position synchronization)
	Distance = 'D',   ///< Distance bars
	Fan = 'f',        ///< Fan/power control (lowercase f, LX200GPS/LX16")
	Focus = 'F',      ///< Focuser control
	GetInfo = 'G',    ///< Get telescope information
	GPS = 'g',        ///< GPS-related commands (lowercase g)
	Home = 'h',       ///< Home position and parking (lowercase h)
	HourFormat = 'H', ///< Hour angle/time format toggle (uppercase H)
	Initialize = 'I', ///< Initialize telescope
	Library = 'L',    ///< Object library commands
	Movement = 'M',   ///< Slew and movement control
	Precision = 'P',  ///< Toggle precision mode
	Quit = 'Q',       ///< Stop/quit movement
	SmartDrive = '@', ///< Smart Drive PEC control (:$Q commands, LX200GPS/LX16")
	Derotator = 'r',  ///< Field de-rotator control (lowercase r, LX16")
	Rate = 'R',       ///< Slew rate control
	SetInfo = 'S',    ///< Set telescope information
	Tracking = 'T',   ///< Tracking rate control
	User = 'U',       ///< User format control
	Waypoint = 'W',   ///< Way point/site commands (LX200GPS)
	Extended = 'X',   ///< Extended OAT-specific commands
	Help = '?',       ///< Help commands (LX200GPS/LX16")
	Unknown = 0       ///< Unrecognized command
};

/**
 * @brief Coordinate precision mode
 *
 * LX200 protocol supports two precision modes for coordinates:
 * - High: HH:MM:SS (arcsecond precision)
 * - Low: HH:MM.T (0.1 arcminute precision)
 */
enum class PrecisionMode : uint8_t {
	High, ///< High precision (HH:MM:SS format)
	Low   ///< Low precision (HH:MM.T format)
};

/**
 * @brief Parse error types
 */
enum class ParseError : uint8_t {
	Incomplete = 1,
	InvalidFormat,
	OutOfRange,
	BufferFull,
	General
};

/**
 * @brief Unit type for void Results
 */
struct Unit {};

/**
 * @brief Functional Result<T> type based on std::variant
 * 
 * Type-safe error handling that replaces error codes.
 */
template<typename T>
class Result {
private:
	std::variant<T, ParseError> data_;
	
public:
	constexpr explicit Result(const T& value) noexcept : data_(value) {}
	constexpr explicit Result(T&& value) noexcept : data_(std::move(value)) {}
	constexpr explicit Result(ParseError error) noexcept : data_(error) {}
	
	constexpr bool is_ok() const noexcept {
		return std::holds_alternative<T>(data_);
	}
	
	constexpr bool is_error() const noexcept {
		return std::holds_alternative<ParseError>(data_);
	}
	
	constexpr const T& value() const {
		return std::get<T>(data_);
	}
	
	constexpr T& value() {
		return std::get<T>(data_);
	}
	
	constexpr ParseError error() const {
		return std::get<ParseError>(data_);
	}
	
	constexpr T value_or(const T& default_value) const {
		return is_ok() ? value() : default_value;
	}
	
	template<typename OkFn, typename ErrFn>
	constexpr auto match(OkFn&& ok_fn, ErrFn&& err_fn) const {
		return is_ok() ? ok_fn(value()) : err_fn(error());
	}
};

/**
 * @brief Result type for void operations
 */
using VoidResult = Result<Unit>;

/**
 * @brief Factory for successful Results
 */
template<typename T>
constexpr Result<std::remove_reference_t<T>> Ok(const T& value) noexcept {
	return Result<std::remove_reference_t<T>>(value);
}

template<typename T>
constexpr Result<std::remove_reference_t<T>> Ok(T&& value) noexcept {
	return Result<std::remove_reference_t<T>>(std::forward<T>(value));
}

constexpr VoidResult Ok() noexcept {
	return VoidResult(Unit{});
}

/**
 * @brief Factory for error Results
 */
template<typename T>
constexpr Result<std::remove_reference_t<T>> Err(ParseError error) noexcept {
	return Result<std::remove_reference_t<T>>(error);
}

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
	uint8_t hours;   ///< 0-23 hours
	uint8_t minutes; ///< 0-59 minutes
	uint8_t seconds; ///< 0-59 arcseconds (HH:MM:SS in high precision, or tenths*6 in low
			 ///< precision)

	/// Compile-time validation
	static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) noexcept
	{
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
	char sign;          ///< '+' or '-'
	uint8_t degrees;    ///< 0-90 degrees
	uint8_t arcminutes; ///< 0-59 arcminutes
	uint8_t arcseconds; ///< 0-59 arcseconds (or 0 in low precision)

	/// Compile-time validation
	static constexpr bool is_valid(char s, uint8_t d, uint8_t m, uint8_t a) noexcept
	{
		return (s == '+' || s == '-') && d <= 90 && m < 60 && a < 60;
	}

	/// Convert to decimal degrees
	constexpr double to_degrees() const noexcept
	{
		double value = degrees + arcminutes / 60.0 + arcseconds / 3600.0;
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
	char sign;          ///< '+' (N) or '-' (S)
	uint8_t degrees;    ///< 0-90 degrees
	uint8_t arcminutes; ///< 0-59 arcminutes

	static constexpr bool is_valid(char s, uint8_t d, uint8_t m) noexcept
	{
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
	uint16_t degrees;   ///< 0-359 degrees
	uint8_t arcminutes; ///< 0-59 arcminutes

	static constexpr bool is_valid(uint16_t d, uint8_t m) noexcept
	{
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
	uint8_t hours;   ///< 0-23 hours
	uint8_t minutes; ///< 0-59 minutes
	uint8_t seconds; ///< 0-59 seconds

	static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) noexcept
	{
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
	uint8_t month; ///< 1-12
	uint8_t day;   ///< 1-31
	uint8_t year;  ///< 0-99 (interpreted as 2000-2099)

	static constexpr bool is_valid(uint8_t m, uint8_t d, uint8_t y) noexcept
	{
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
 * references to the parser's internal buffer.
 *
 * @warning LIFETIME SAFETY: The string_view members reference the
 * parser's internal buffer. This Command is only valid until the
 * next call to feed_character() or reset(). You MUST consume the
 * command (extract name/parameters) before feeding the next character.
 * Failure to do so will result in use-after-invalidation.
 *
 * @par Usage Pattern (SAFE):
 * @code
 * auto cmd = parser.get_command();
 * if (cmd) {
 *     std::string name(cmd->name);         // Copy before next feed
 *     std::string params(cmd->parameters); // Copy before next feed
 *     parser.feed_character(':');          // Now safe to feed next
 * }
 * @endcode
 *
 * @par Usage Pattern (UNSAFE):
 * @code
 * auto cmd = parser.get_command();         // cmd holds views into buffer
 * parser.feed_character(':');              // DANGER: buffer reset!
 * if (cmd) {
 *     // cmd->name is now DANGLING - use-after-free!
 * }
 * @endcode
 */
struct Command {
	CommandFamily family;        ///< Command family classification
	std::string_view name;       ///< Full command name (e.g., "GR", "Sr")
	std::string_view parameters; ///< Parameter substring (empty if none)

	/// Check if command has parameters
	constexpr bool has_parameters() const noexcept
	{
		return !parameters.empty();
	}

	/// Get first character of command (after ':')
	constexpr char first_char() const noexcept
	{
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
class ParserState
{
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
	 * @return VoidResult
	 *
	 * Returns Ok() when character accepted (including terminator).
	 * Check is_command_ready() after Ok() to see if command complete.
	 * Returns Err() for invalid input or buffer full.
	 */
	VoidResult feed_character(char c) noexcept;

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
	 * After calling, parser resets its internal buffer to accept the
	 * next command.
	 *
	 * @warning LIFETIME SAFETY: The returned Command contains string_view
	 * references into the parser's internal buffer. These views become
	 * INVALID as soon as you call feed_character() again. You MUST copy
	 * any needed strings before feeding the next character.
	 *
	 * @par Safe Usage:
	 * @code
	 * if (auto cmd = parser.get_command()) {
	 *     // Copy strings before next feed:
	 *     std::string name(cmd->name);
	 *     std::string params(cmd->parameters);
	 *     // Now safe to continue:
	 *     parser.feed_character(':');
	 * }
	 * @endcode
	 */
	std::optional<Command> get_command() noexcept;

	/**
	 * @brief Get current precision mode
	 */
	PrecisionMode get_precision() const noexcept
	{
		return precision_;
	}

	/**
	 * @brief Set precision mode
	 *
	 * @param mode New precision mode
	 */
	void set_precision(PrecisionMode mode) noexcept
	{
		precision_ = mode;
	}

	/**
	 * @brief Get maximum command buffer length
	 *
	 * Exposes the internal buffer size limit for testing and validation.
	 *
	 * @return Maximum number of characters (including ':' prefix and '#' terminator)
	 */
	static constexpr size_t max_command_length() noexcept
	{
		return MAX_COMMAND_LENGTH;
	}

      private:
	static constexpr size_t MAX_COMMAND_LENGTH = 64; ///< Maximum command buffer size

	std::array<char, MAX_COMMAND_LENGTH> buffer_;  ///< Command buffer
	size_t buffer_length_{0};                      ///< Current buffer length
	bool command_complete_{false};                 ///< Command complete flag
	PrecisionMode precision_{PrecisionMode::High}; ///< Current precision mode

	/// Identify command family from command name (handles both single-char and $-prefixed)
	CommandFamily identify_family(std::string_view name) const noexcept;

	/// Split command name from parameters
	void parse_command_parts(std::string_view &name, std::string_view &params) const noexcept;
};

/* ========================================================================
 * Functional Parsing API - Result<T> based
 * ======================================================================== */

/**
 * @brief Parse Right Ascension coordinate string
 *
 * @param str String in format HH:MM:SS or HH:MM.T
 * @param mode Precision mode (High or Low)
 * @return Result<RACoordinate> containing parsed coordinate or error
 */
Result<RACoordinate> parse_ra(std::string_view str, PrecisionMode mode) noexcept;

/**
 * @brief Parse Declination coordinate string
 *
 * @param str String in format sDD*MM:SS or sDD*MM
 * @param mode Precision mode (High or Low)
 * @return Result<DECCoordinate> containing parsed coordinate or error
 */
Result<DECCoordinate> parse_dec(std::string_view str, PrecisionMode mode) noexcept;

/**
 * @brief Parse Latitude coordinate string
 *
 * @param str String in format sDD*MM
 * @return Result<LatitudeCoordinate> containing parsed coordinate or error
 */
Result<LatitudeCoordinate> parse_latitude(std::string_view str) noexcept;

/**
 * @brief Parse Longitude coordinate string
 *
 * @param str String in format DDD*MM
 * @return Result<LongitudeCoordinate> containing parsed coordinate or error
 */
Result<LongitudeCoordinate> parse_longitude(std::string_view str) noexcept;

/**
 * @brief Parse Time value string
 *
 * @param str String in format HH:MM:SS
 * @return Result<TimeValue> containing parsed time or error
 */
Result<TimeValue> parse_time(std::string_view str) noexcept;

/**
 * @brief Parse Date value string
 *
 * @param str String in format MM/DD/YY
 * @return Result<DateValue> containing parsed date or error
 */
Result<DateValue> parse_date(std::string_view str) noexcept;

} // namespace lx200

#endif // LX200_HPP

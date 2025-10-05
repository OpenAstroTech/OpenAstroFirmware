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
 * Classification mapping:
 * - Single-character families: A, B, C, D, f, F, G, g, h, H, I, L, M, P, Q, r, R, S, T, U, W, X, ?
 * - Two-character families: $B (Backlash), $Q (SmartDrive)
 * 
 * Note: This is purely a type-safe categorization enum. The actual character
 * mapping is handled by identify_family() which inspects the command string.
 * Enum values are auto-assigned by the compiler for simplicity.
 */
enum class CommandFamily : uint8_t {
	Alignment,    ///< Telescope alignment commands (A)
	Backlash,     ///< Active backlash compensation ($B commands, LX200GPS)
	Reticle,      ///< Reticle brightness and accessory control (B)
	Sync,         ///< Sync control (C - telescope position synchronization)
	Distance,     ///< Distance bars (D)
	Fan,          ///< Fan/power control (f - lowercase, LX200GPS/LX16")
	Focus,        ///< Focuser control (F)
	GetInfo,      ///< Get telescope information (G - uppercase)
	GPS,          ///< GPS-related commands (g - lowercase)
	Home,         ///< Home position and parking (h - lowercase)
	HourFormat,   ///< Hour angle/time format toggle (H - uppercase)
	Initialize,   ///< Initialize telescope (I)
	Library,      ///< Object library commands (L)
	Movement,     ///< Slew and movement control (M)
	Precision,    ///< Toggle precision mode (P)
	Quit,         ///< Stop/quit movement (Q)
	SmartDrive,   ///< Smart Drive PEC control ($Q commands, LX200GPS/LX16")
	Derotator,    ///< Field de-rotator control (r - lowercase, LX16")
	Rate,         ///< Slew rate control (R)
	SetInfo,      ///< Set telescope information (S)
	Tracking,     ///< Tracking rate control (T)
	User,         ///< User format control (U)
	Waypoint,     ///< Way point/site commands (W - LX200GPS)
	Extended,     ///< Extended OAT-specific commands (X)
	Help,         ///< Help commands (? - LX200GPS/LX16")
	Unknown       ///< Unrecognized command
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
 * Provides Rust-style error handling without exceptions or error codes.
 * A Result<T> either contains a successful value (Ok) or an error (Err).
 * 
 * Key Benefits:
 * - Type-safe: Cannot accidentally ignore errors
 * - Composable: Chain operations with match()
 * - Zero-cost: No exceptions or dynamic allocation
 * - Explicit: Error handling is visible in function signatures
 * 
 * @par Basic Usage:
 * @code
 * // Function returns Result instead of throwing or returning error codes
 * Result<RACoordinate> coord = parse_ra("12:34:56", PrecisionMode::High);
 * 
 * // Check if successful
 * if (coord.is_ok()) {
 *     RACoordinate ra = coord.value();
 *     LOG_INF("RA: %02d:%02d:%02d", ra.hours, ra.minutes, ra.seconds);
 * } else {
 *     ParseError err = coord.error();
 *     LOG_ERR("Parse failed: %d", static_cast<int>(err));
 * }
 * @endcode
 * 
 * @par Using value_or() for Defaults:
 * @code
 * // Provide fallback value on error
 * RACoordinate ra = parse_ra(input, mode).value_or(RACoordinate{0, 0, 0});
 * @endcode
 * 
 * @par Pattern Matching with match():
 * @code
 * // Handle both cases functionally
 * auto message = parse_ra(input, mode).match(
 *     [](const RACoordinate& ra) {
 *         return format_ra(ra);  // Success path
 *     },
 *     [](ParseError err) {
 *         return std::string("Invalid");  // Error path
 *     }
 * );
 * @endcode
 * 
 * @par Chaining Operations:
 * @code
 * // Parse and validate in one expression
 * auto result = parse_ra(input, mode).match(
 *     [](const RACoordinate& ra) -> Result<std::string> {
 *         if (RACoordinate::is_valid(ra.hours, ra.minutes, ra.seconds)) {
 *             return Ok(format_ra(ra));
 *         }
 *         return Err<std::string>(ParseError::OutOfRange);
 *     },
 *     [](ParseError err) -> Result<std::string> {
 *         return Err<std::string>(err);
 *     }
 * );
 * @endcode
 * 
 * @par Creating Results:
 * @code
 * // Use factory functions for clarity
 * Result<int> success = Ok(42);
 * Result<int> failure = Err<int>(ParseError::InvalidFormat);
 * VoidResult void_ok = Ok();  // For void operations
 * VoidResult void_err = Err<Unit>(ParseError::General);
 * @endcode
 * 
 * @tparam T The type of the success value
 * 
 * @see Ok() - Factory for successful Results
 * @see Err() - Factory for error Results
 * @see VoidResult - Typedef for Result<Unit> (void operations)
 */
template<typename T>
class Result {
private:
	std::variant<T, ParseError> data_;
	
public:
	/// Construct Result with success value (copy)
	constexpr explicit Result(const T& value) noexcept : data_(value) {}
	
	/// Construct Result with success value (move)
	constexpr explicit Result(T&& value) noexcept : data_(std::move(value)) {}
	
	/// Construct Result with error
	constexpr explicit Result(ParseError error) noexcept : data_(error) {}
	
	/**
	 * @brief Check if Result contains a success value
	 * @return true if Ok, false if Err
	 */
	constexpr bool is_ok() const noexcept {
		return std::holds_alternative<T>(data_);
	}
	
	/**
	 * @brief Check if Result contains an error
	 * @return true if Err, false if Ok
	 */
	constexpr bool is_error() const noexcept {
		return std::holds_alternative<ParseError>(data_);
	}
	
	/**
	 * @brief Extract the success value (const)
	 * @return Reference to the contained value
	 * @throws std::bad_variant_access if Result is an error
	 * @note Always check is_ok() before calling, or use value_or() for safety
	 */
	constexpr const T& value() const {
		return std::get<T>(data_);
	}
	
	/**
	 * @brief Extract the success value (mutable)
	 * @return Mutable reference to the contained value
	 * @throws std::bad_variant_access if Result is an error
	 * @note Always check is_ok() before calling, or use value_or() for safety
	 */
	constexpr T& value() {
		return std::get<T>(data_);
	}
	
	/**
	 * @brief Extract the error value
	 * @return The contained ParseError
	 * @throws std::bad_variant_access if Result is Ok
	 * @note Always check is_error() before calling
	 */
	constexpr ParseError error() const {
		return std::get<ParseError>(data_);
	}
	
	/**
	 * @brief Extract value or return default on error
	 * @param default_value Value to return if Result is an error
	 * @return The contained value if Ok, or default_value if Err
	 * 
	 * @par Example:
	 * @code
	 * // Safe extraction without checking
	 * RACoordinate ra = parse_ra(input, mode).value_or(RACoordinate{0, 0, 0});
	 * @endcode
	 */
	constexpr T value_or(const T& default_value) const {
		return is_ok() ? value() : default_value;
	}
	
	/**
	 * @brief Pattern match on Result with callbacks
	 * @param ok_fn Callback for success case: T -> R
	 * @param err_fn Callback for error case: ParseError -> R
	 * @return Result of calling the appropriate callback
	 * 
	 * This is the functional way to handle Results, similar to Rust's match.
	 * Both callbacks must return the same type R.
	 * 
	 * @par Example - Simple Logging:
	 * @code
	 * parse_ra(input, mode).match(
	 *     [](const RACoordinate& ra) {
	 *         LOG_INF("Parsed: %02d:%02d:%02d", ra.hours, ra.minutes, ra.seconds);
	 *     },
	 *     [](ParseError err) {
	 *         LOG_ERR("Parse error: %d", static_cast<int>(err));
	 *     }
	 * );
	 * @endcode
	 * 
	 * @par Example - Transforming Results:
	 * @code
	 * std::string message = parse_ra(input, mode).match(
	 *     [](const RACoordinate& ra) { return format_ra(ra); },
	 *     [](ParseError err) { return std::string("ERROR"); }
	 * );
	 * @endcode
	 */
	template<typename OkFn, typename ErrFn>
	constexpr auto match(OkFn&& ok_fn, ErrFn&& err_fn) const {
		return is_ok() ? ok_fn(value()) : err_fn(error());
	}
};

/**
 * @brief Result type for void operations
 * 
 * Use this for functions that can fail but don't return a value.
 * Equivalent to Result<Unit>.
 * 
 * @par Example:
 * @code
 * VoidResult validate_input(const char* input) {
 *     if (input == nullptr) {
 *         return Err<Unit>(ParseError::InvalidFormat);
 *     }
 *     return Ok();  // Success with no value
 * }
 * @endcode
 */
using VoidResult = Result<Unit>;

/**
 * @brief Factory for successful Results
 * 
 * Creates a Result<T> containing a success value. Use this instead of
 * calling Result<T> constructor directly for better type inference.
 * 
 * @param value The success value to wrap
 * @return Result<T> containing the value
 * 
 * @par Example - With Value:
 * @code
 * Result<int> parse_number(const char* str) {
 *     int val = atoi(str);
 *     if (val == 0 && str[0] != '0') {
 *         return Err<int>(ParseError::InvalidFormat);
 *     }
 *     return Ok(val);  // Type deduced as Result<int>
 * }
 * @endcode
 * 
 * @par Example - Void (No Value):
 * @code
 * VoidResult check_bounds(int value) {
 *     if (value < 0 || value > 100) {
 *         return Err<Unit>(ParseError::OutOfRange);
 *     }
 *     return Ok();  // Success with no value
 * }
 * @endcode
 */
template<typename T>
constexpr Result<std::remove_reference_t<T>> Ok(const T& value) noexcept {
	return Result<std::remove_reference_t<T>>(value);
}

/// Factory for successful Results (move version)
template<typename T>
constexpr Result<std::remove_reference_t<T>> Ok(T&& value) noexcept {
	return Result<std::remove_reference_t<T>>(std::forward<T>(value));
}

/// Factory for successful void Results
constexpr VoidResult Ok() noexcept {
	return VoidResult(Unit{});
}

/**
 * @brief Factory for error Results
 * 
 * Creates a Result<T> containing an error. The type T must be explicitly
 * specified since it cannot be inferred from the error value.
 * 
 * @tparam T The success type (even though we're creating an error)
 * @param error The error to wrap
 * @return Result<T> containing the error
 * 
 * @par Example:
 * @code
 * Result<RACoordinate> parse_ra(const char* str) {
 *     if (str == nullptr) {
 *         return Err<RACoordinate>(ParseError::InvalidFormat);
 *     }
 *     // ... parsing logic ...
 *     return Ok(coordinate);
 * }
 * @endcode
 * 
 * @note Type must be specified: Err<int>(error), not Err(error)
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

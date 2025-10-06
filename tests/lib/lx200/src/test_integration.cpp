/*
 * LX200 Integration Tests - Result<T> API
 * End-to-end tests demonstrating Result<T> patterns and workflows
 */

#include <lx200/lx200.hpp>
#include <zephyr/ztest.h>

using namespace lx200;

/* ========================================================================
 * Parser + Coordinate Integration Tests
 * ======================================================================== */

ZTEST(lx200_integration, test_parse_set_ra_command)
{
	ParserState parser;

	// Parse command ":Sr12:34:56#"
	const char *cmd = ":Sr12:34:56#";
	for (const char *p = cmd; *p != '\0'; p++) {
		auto result = parser.feed_character(*p);
		if (result.is_error()) {
			zassert_unreachable("Parser should not error");
		}
	}

	zassert_true(parser.is_command_ready(), "Command should be ready");

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value(), "Should have command");
	auto command = command_opt.value();

	zassert_equal(command.name[0], 'S', "Command family should be 'S'");
	zassert_equal(command.name[1], 'r', "Command should be 'r'");

	// Extract parameter
	std::string_view param = command.parameters;

	// Parse the coordinate
	auto coord_result = parse_ra(param, PrecisionMode::High);
	zassert_true(coord_result.is_ok(), "Should parse RA coordinate");

	auto coord = coord_result.value();
	zassert_equal(coord.hours, 12);
	zassert_equal(coord.minutes, 34);
	zassert_equal(coord.seconds, 56);
}

ZTEST(lx200_integration, test_parse_set_dec_command)
{
	ParserState parser;

	// Parse command ":Sd+45*30:15#"
	const char *cmd = ":Sd+45*30:15#";
	for (const char *p = cmd; *p != '\0'; p++) {
		auto result = parser.feed_character(*p);
		if (result.is_error()) {
			zassert_unreachable("Parser should not error");
		}
	}

	zassert_true(parser.is_command_ready(), "Command should be ready");

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	zassert_equal(command.name[0], 'S', "Command family should be 'S'");
	zassert_equal(command.name[1], 'd', "Command should be 'd'");

	// Extract parameter
	std::string_view param = command.parameters;

	// Parse the coordinate
	auto coord_result = parse_dec(param, PrecisionMode::High);
	zassert_true(coord_result.is_ok(), "Should parse DEC coordinate");

	auto coord = coord_result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 45);
	zassert_equal(coord.arcminutes, 30);
	zassert_equal(coord.arcseconds, 15);
}

ZTEST(lx200_integration, test_parse_set_site_latitude)
{
	ParserState parser;

	// Parse command ":St+37*23#"
	const char *cmd = ":St+37*23#";
	for (const char *p = cmd; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	std::string_view param = command.parameters;

	auto coord_result = parse_latitude(param);
	zassert_true(coord_result.is_ok(), "Should parse latitude");

	auto coord = coord_result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 37);
	zassert_equal(coord.arcminutes, 23);
}

ZTEST(lx200_integration, test_parse_set_site_longitude)
{
	ParserState parser;

	// Parse command ":Sg122*04#" (longitude has no sign)
	const char *cmd = ":Sg122*04#";
	for (const char *p = cmd; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	std::string_view param = command.parameters;

	auto coord_result = parse_longitude(param);
	zassert_true(coord_result.is_ok(), "Should parse longitude");

	auto coord = coord_result.value();
	zassert_equal(coord.degrees, 122);
	zassert_equal(coord.arcminutes, 4);
}

ZTEST(lx200_integration, test_parse_set_local_time)
{
	ParserState parser;

	// Parse command ":SL14:30:45#"
	const char *cmd = ":SL14:30:45#";
	for (const char *p = cmd; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	std::string_view param = command.parameters;

	auto time_result = parse_time(param);
	zassert_true(time_result.is_ok(), "Should parse time");

	auto time = time_result.value();
	zassert_equal(time.hours, 14);
	zassert_equal(time.minutes, 30);
	zassert_equal(time.seconds, 45);
}

ZTEST(lx200_integration, test_parse_set_calendar_date)
{
	ParserState parser;

	// Parse command ":SC12/25/23#"
	const char *cmd = ":SC12/25/23#";
	for (const char *p = cmd; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	std::string_view param = command.parameters;

	auto date_result = parse_date(param);
	zassert_true(date_result.is_ok(), "Should parse date");

	auto date = date_result.value();
	zassert_equal(date.month, 12);
	zassert_equal(date.day, 25);
	zassert_equal(date.year, 23);
}

/* ========================================================================
 * Error Handling Integration Tests
 * ======================================================================== */

ZTEST(lx200_integration, test_parser_error_propagation)
{
	ParserState parser;

	// Try to parse invalid command (no start marker)
	auto result = parser.feed_character('G');

	zassert_true(result.is_error(), "Should error immediately");
	zassert_equal(result.error(), ParseError::InvalidFormat);

	// Parser should still be in error state
	zassert_false(parser.is_command_ready(), "No valid command");
}

ZTEST(lx200_integration, test_coordinate_error_in_command)
{
	ParserState parser;

	// Parse command with invalid RA ":Sr24:00:00#"
	const char *cmd = ":Sr24:00:00#";
	for (const char *p = cmd; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	std::string_view param = command.parameters;

	// Try to parse the invalid coordinate
	auto coord_result = parse_ra(param, PrecisionMode::High);

	zassert_true(coord_result.is_error(), "Should error on invalid RA");
	zassert_equal(coord_result.error(), ParseError::OutOfRange);
}

/* ========================================================================
 * Result<T> Pattern Demonstrations
 * ======================================================================== */

ZTEST(lx200_integration, test_result_chaining_with_match)
{
	auto ra_result = parse_ra("12:34:56", PrecisionMode::High);
	auto dec_result = parse_dec("+45*30:15", PrecisionMode::High);

	// Both should succeed
	bool both_ok = ra_result.is_ok() && dec_result.is_ok();
	zassert_true(both_ok, "Both coordinates should parse");

	// Use match to extract values
	RACoordinate ra{};
	ra_result.match([&ra](const RACoordinate &coord) { ra = coord; },
			[](ParseError) { zassert_unreachable("RA should not error"); });

	DECCoordinate dec{};
	dec_result.match([&dec](const DECCoordinate &coord) { dec = coord; },
			 [](ParseError) { zassert_unreachable("DEC should not error"); });

	// Verify extracted values
	zassert_equal(ra.hours, 12);
	zassert_equal(dec.degrees, 45);
}

ZTEST(lx200_integration, test_result_error_handling_with_match)
{
	auto result = parse_ra("invalid", PrecisionMode::High);

	bool error_handled = false;
	ParseError captured_error = ParseError::General;

	result.match([](const RACoordinate &) { zassert_unreachable("Should not succeed"); },
		     [&](ParseError err) {
			     error_handled = true;
			     captured_error = err;
		     });

	zassert_true(error_handled, "Error callback should be called");
	zassert_equal(captured_error, ParseError::InvalidFormat);
}

ZTEST(lx200_integration, test_result_value_or_with_fallback)
{
	// Invalid parse
	auto error_result = parse_time("99:99:99");

	// Provide fallback
	TimeValue fallback{12, 0, 0};
	auto time = error_result.value_or(fallback);

	zassert_equal(time.hours, 12, "Should use fallback hours");
	zassert_equal(time.minutes, 0, "Should use fallback minutes");
	zassert_equal(time.seconds, 0, "Should use fallback seconds");

	// Valid parse should not use fallback
	auto valid_result = parse_time("14:30:45");
	time = valid_result.value_or(fallback);

	zassert_equal(time.hours, 14, "Should use parsed hours");
	zassert_equal(time.minutes, 30, "Should use parsed minutes");
	zassert_equal(time.seconds, 45, "Should use parsed seconds");
}

ZTEST(lx200_integration, test_multiple_coordinate_validation)
{
	struct SiteLocation {
		LatitudeCoordinate lat;
		LongitudeCoordinate lon;
	};

	auto lat_result = parse_latitude("+37*23");
	auto lon_result = parse_longitude("122*04"); // Longitude has no sign

	if (lat_result.is_ok() && lon_result.is_ok()) {
		SiteLocation loc = {lat_result.value(), lon_result.value()};

		zassert_equal(loc.lat.degrees, 37);
		zassert_equal(loc.lon.degrees, 122);
	} else {
		zassert_unreachable("Both coordinates should parse");
	}
}

ZTEST(lx200_integration, test_precision_mode_switching)
{
	const char *ra_str = "12:34.5"; // Could be low or high precision format

	// Try high precision first (will fail for this format)
	auto high_result = parse_ra(ra_str, PrecisionMode::High);

	if (high_result.is_error()) {
		// Fall back to low precision
		auto low_result = parse_ra(ra_str, PrecisionMode::Low);

		if (low_result.is_ok()) {
			auto coord = low_result.value();
			zassert_equal(coord.hours, 12);
			zassert_equal(coord.minutes, 34);
			// Note: .5 tenths = 30 seconds
		}
	}
}

ZTEST(lx200_integration, test_void_result_from_parser)
{
	ParserState parser;

	// VoidResult indicates success/failure without a value
	VoidResult result = parser.feed_character(':');

	zassert_true(result.is_ok(), "Should succeed");

	// Can use match on VoidResult too
	bool success = false;
	result.match([&](const Unit &) { success = true; },
		     [](ParseError) { zassert_unreachable("Should not error"); });

	zassert_true(success, "Match should work on VoidResult");
}

/* ========================================================================
 * Real-World Command Scenarios
 * ======================================================================== */

ZTEST(lx200_integration, test_slew_to_target_workflow)
{
	ParserState parser;

	// Step 1: Set target RA
	const char *set_ra = ":Sr12:34:56#";
	for (const char *p = set_ra; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto ra_cmd_opt = parser.get_command();
	zassert_true(ra_cmd_opt.has_value());
	std::string_view ra_param = ra_cmd_opt->parameters;
	auto ra_result = parse_ra(ra_param, PrecisionMode::High);
	zassert_true(ra_result.is_ok(), "RA should parse");

	parser.reset();

	// Step 2: Set target DEC
	const char *set_dec = ":Sd+45*30:15#";
	for (const char *p = set_dec; *p != '\0'; p++) {
		parser.feed_character(*p);
	}

	auto dec_cmd_opt = parser.get_command();
	zassert_true(dec_cmd_opt.has_value());
	std::string_view dec_param = dec_cmd_opt->parameters;
	auto dec_result = parse_dec(dec_param, PrecisionMode::High);
	zassert_true(dec_result.is_ok(), "DEC should parse");

	// Both coordinates valid - ready to slew
	if (ra_result.is_ok() && dec_result.is_ok()) {
		auto ra = ra_result.value();
		auto dec = dec_result.value();

		// In real code, would initiate slew here
		zassert_equal(ra.hours, 12);
		zassert_equal(dec.degrees, 45);
	}
}

ZTEST(lx200_integration, test_site_setup_workflow)
{
	// Simulate setting up site location
	auto lat_result = parse_latitude("+37*23");
	auto lon_result =
		parse_longitude("122*04"); // Longitude has no sign (0-359 west from Greenwich)

	// Both must succeed for valid site
	if (!lat_result.is_ok() || !lon_result.is_ok()) {
		zassert_unreachable("Site coordinates should parse");
	}

	auto lat = lat_result.value();
	auto lon = lon_result.value();

	// Verify site is in Northern Hemisphere
	zassert_equal(lat.sign, '+', "Should be North");
	// Longitude is 0-359 degrees west from Greenwich (no sign)
	zassert_true(lon.degrees < 360, "Longitude should be valid");
}

ZTEST(lx200_integration, test_time_and_date_setup)
{
	// Set local time and date
	auto time_result = parse_time("14:30:45");
	auto date_result = parse_date("12/25/23");

	if (time_result.is_ok() && date_result.is_ok()) {
		auto time = time_result.value();
		auto date = date_result.value();

		// Verify time: 2:30:45 PM
		zassert_equal(time.hours, 14);
		zassert_equal(time.minutes, 30);

		// Verify date: December 25, 2023
		zassert_equal(date.month, 12);
		zassert_equal(date.day, 25);
		zassert_equal(date.year, 23);
	}
}

ZTEST_SUITE(lx200_integration, NULL, NULL, NULL, NULL, NULL);

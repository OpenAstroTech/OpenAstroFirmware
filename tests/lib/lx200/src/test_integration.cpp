/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <lx200/lx200.hpp>
#include <string>

/**
 * @file test_integration.cpp
 * @brief LX200 Integration Tests
 *
 * End-to-end tests simulating real telescope control scenarios:
 * - Setting target coordinates and slewing
 * - Querying current position
 * - Precision mode changes
 * - Site location and time configuration
 * - Movement control sequences
 */

using namespace lx200;

/* ========================================================================
 * Slew Command Integration Tests
 * ======================================================================== */

/**
 * @brief Test complete slew-to-target sequence
 *
 * Scenario: Planetarium software wants to slew to M31 (Andromeda Galaxy)
 * - Set target RA to 00:42:44
 * - Set target DEC to +41*16:09
 * - Initiate slew to target
 */
ZTEST(lx200, test_slew_to_target_sequence)
{
	ParserState parser;

	// Step 1: Set target RA (:Sr00:42:44#)
	for (const char c : std::string_view(":Sr00:42:44#")) {
		parser.feed_character(c);
	}
	auto cmd1 = parser.get_command();
	zassert_true(cmd1.has_value(), "Should parse Sr command");
	zassert_equal(cmd1->family, CommandFamily::SetInfo);
	zassert_mem_equal(cmd1->name.data(), "Sr", 2);

	// Parse RA parameter
	RACoordinate ra;
	auto ra_result =
		parse_ra_coordinate(std::string(cmd1->parameters).c_str(), PrecisionMode::High, ra);
	zassert_equal(ra_result, ParseResult::Success, "Should parse RA parameters");
	zassert_equal(ra.hours, 0);
	zassert_equal(ra.minutes, 42);
	zassert_equal(ra.seconds, 44);

	// Step 2: Set target DEC (:Sd+41*16:09#)
	parser.reset();
	for (const char c : std::string_view(":Sd+41*16:09#")) {
		parser.feed_character(c);
	}
	auto cmd2 = parser.get_command();
	zassert_true(cmd2.has_value(), "Should parse Sd command");
	zassert_equal(cmd2->family, CommandFamily::SetInfo);

	// Parse DEC parameter
	DECCoordinate dec;
	auto dec_result = parse_dec_coordinate(std::string(cmd2->parameters).c_str(),
					       PrecisionMode::High, dec);
	zassert_equal(dec_result, ParseResult::Success, "Should parse DEC parameters");
	zassert_equal(dec.sign, '+');
	zassert_equal(dec.degrees, 41);
	zassert_equal(dec.arcminutes, 16);
	zassert_equal(dec.arcseconds, 9);

	// Step 3: Initiate slew (:MS#)
	parser.reset();
	for (const char c : std::string_view(":MS#")) {
		parser.feed_character(c);
	}
	auto cmd3 = parser.get_command();
	zassert_true(cmd3.has_value(), "Should parse MS command");
	zassert_equal(cmd3->family, CommandFamily::Movement);
}

/* ========================================================================
 * Position Query Integration Tests
 * ======================================================================== */

/**
 * @brief Test querying current telescope position
 *
 * Scenario: Software queries current RA/DEC coordinates
 * - Get current RA (:GR#)
 * - Get current DEC (:GD#)
 */
ZTEST(lx200, test_position_query_sequence)
{
	ParserState parser;

	// Query RA
	for (const char c : std::string_view(":GR#")) {
		parser.feed_character(c);
	}
	auto cmd1 = parser.get_command();
	zassert_true(cmd1.has_value(), "Should parse GR command");
	zassert_equal(cmd1->family, CommandFamily::GetInfo);
	zassert_mem_equal(cmd1->name.data(), "GR", 2);

	// Query DEC
	parser.reset();
	for (const char c : std::string_view(":GD#")) {
		parser.feed_character(c);
	}
	auto cmd2 = parser.get_command();
	zassert_true(cmd2.has_value(), "Should parse GD command");
	zassert_equal(cmd2->family, CommandFamily::GetInfo);
	zassert_mem_equal(cmd2->name.data(), "GD", 2);
}

/* ========================================================================
 * Precision Mode Integration Tests
 * ======================================================================== */

/**
 * @brief Test precision mode toggle affects coordinate parsing
 *
 * Scenario: Switch between high and low precision coordinate formats
 */
ZTEST(lx200, test_precision_mode_integration)
{
	ParserState parser;

	// Start in High precision (default)
	zassert_equal(parser.get_precision(), PrecisionMode::High);

	// Toggle to Low precision (:P#)
	for (const char c : std::string_view(":P#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse P command");

	// Simulate toggle action
	parser.set_precision(PrecisionMode::Low);
	zassert_equal(parser.get_precision(), PrecisionMode::Low);

	// Now parsing should expect low precision format
	// :Sr12:34.5# (low precision RA)
	parser.reset();
	for (const char c : std::string_view(":Sr12:34.5#")) {
		parser.feed_character(c);
	}
	auto cmd2 = parser.get_command();
	zassert_true(cmd2.has_value(), "Should parse low precision Sr");

	RACoordinate ra;
	auto result =
		parse_ra_coordinate(std::string(cmd2->parameters).c_str(), PrecisionMode::Low, ra);
	zassert_equal(result, ParseResult::Success, "Should parse low precision RA");
}

/* ========================================================================
 * Site Configuration Integration Tests
 * ======================================================================== */

/**
 * @brief Test site location and time configuration sequence
 *
 * Scenario: Configure telescope for San Francisco observing session
 * - Set latitude +37*45
 * - Set longitude 122*30
 * - Set local time 21:30:00
 * - Set date 03/15/23
 */
ZTEST(lx200, test_site_configuration_sequence)
{
	ParserState parser;

	// Set latitude (:St+37*45#)
	for (const char c : std::string_view(":St+37*45#")) {
		parser.feed_character(c);
	}
	auto cmd1 = parser.get_command();
	zassert_true(cmd1.has_value(), "Should parse St command");

	LatitudeCoordinate lat;
	auto lat_result = parse_latitude_coordinate(std::string(cmd1->parameters).c_str(), lat);
	zassert_equal(lat_result, ParseResult::Success, "Should parse latitude");
	zassert_equal(lat.degrees, 37);

	// Set longitude (:Sg122*30#)
	parser.reset();
	for (const char c : std::string_view(":Sg122*30#")) {
		parser.feed_character(c);
	}
	auto cmd2 = parser.get_command();
	zassert_true(cmd2.has_value(), "Should parse Sg command");

	LongitudeCoordinate lon;
	auto lon_result = parse_longitude_coordinate(std::string(cmd2->parameters).c_str(), lon);
	zassert_equal(lon_result, ParseResult::Success, "Should parse longitude");
	zassert_equal(lon.degrees, 122);

	// Set local time (:SL21:30:00#)
	parser.reset();
	for (const char c : std::string_view(":SL21:30:00#")) {
		parser.feed_character(c);
	}
	auto cmd3 = parser.get_command();
	zassert_true(cmd3.has_value(), "Should parse SL command");

	TimeValue time;
	auto time_result = parse_time_value(std::string(cmd3->parameters).c_str(), time);
	zassert_equal(time_result, ParseResult::Success, "Should parse time");
	zassert_equal(time.hours, 21);
	zassert_equal(time.minutes, 30);

	// Set date (:SC03/15/23#)
	parser.reset();
	for (const char c : std::string_view(":SC03/15/23#")) {
		parser.feed_character(c);
	}
	auto cmd4 = parser.get_command();
	zassert_true(cmd4.has_value(), "Should parse SC command");

	DateValue date;
	auto date_result = parse_date_value(std::string(cmd4->parameters).c_str(), date);
	zassert_equal(date_result, ParseResult::Success, "Should parse date");
	zassert_equal(date.month, 3);
	zassert_equal(date.day, 15);
}

/* ========================================================================
 * Movement Control Integration Tests
 * ======================================================================== */

/**
 * @brief Test manual movement control sequence
 *
 * Scenario: User manually moves telescope east, then stops
 * - Set slew rate (:RS#)
 * - Start moving east (:Me#)
 * - Stop all movement (:Q#)
 */
ZTEST(lx200, test_movement_control_sequence)
{
	ParserState parser;

	// Set slew rate
	for (const char c : std::string_view(":RS#")) {
		parser.feed_character(c);
	}
	auto cmd1 = parser.get_command();
	zassert_true(cmd1.has_value(), "Should parse RS command");
	zassert_equal(cmd1->family, CommandFamily::Rate);

	// Move east
	parser.reset();
	for (const char c : std::string_view(":Me#")) {
		parser.feed_character(c);
	}
	auto cmd2 = parser.get_command();
	zassert_true(cmd2.has_value(), "Should parse Me command");
	zassert_equal(cmd2->family, CommandFamily::Movement);

	// Stop movement
	parser.reset();
	for (const char c : std::string_view(":Q#")) {
		parser.feed_character(c);
	}
	auto cmd3 = parser.get_command();
	zassert_true(cmd3.has_value(), "Should parse Q command");
	zassert_equal(cmd3->family, CommandFamily::Quit);
}

/* ========================================================================
 * Error Recovery Integration Tests
 * ======================================================================== */

/**
 * @brief Test parser recovers from errors in command stream
 *
 * Scenario: Noisy serial line causes parsing errors, but valid
 * commands should still be processed
 */
ZTEST(lx200, test_error_recovery)
{
	ParserState parser;

	// Invalid command (missing prefix)
	auto result1 = parser.feed_character('G');
	zassert_equal(result1, ParseResult::ErrorInvalidFormat, "Should detect error");

	// Reset and try valid command
	parser.reset();
	for (const char c : std::string_view(":GR#")) {
		auto result = parser.feed_character(c);
		zassert_true(result == ParseResult::Incomplete || result == ParseResult::Success,
			     "Should accept valid characters");
	}

	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse valid command after error recovery");
}

/* ========================================================================
 * Rapid Command Sequence Tests
 * ======================================================================== */

/**
 * @brief Test parser handles rapid command sequences
 *
 * Scenario: Software sends multiple commands quickly
 */
ZTEST(lx200, test_rapid_command_sequence)
{
	ParserState parser;
	const char *commands[] = {
		":GR#", // Get RA
		":GD#", // Get DEC
		":GG#", // Get UTC offset
		":Gg#", // Get current site longitude
		":Gt#", // Get current site latitude
	};

	for (const char *cmd_str : commands) {
		parser.reset();
		for (const char *p = cmd_str; *p != '\0'; p++) {
			auto result = parser.feed_character(*p);
			zassert_true(result == ParseResult::Incomplete ||
					     result == ParseResult::Success,
				     "Should accept all characters");
		}

		zassert_true(parser.is_command_ready(), "Command should be ready");

		auto cmd = parser.get_command();
		zassert_true(cmd.has_value(), "Should return command");
		zassert_equal(cmd->family, CommandFamily::GetInfo,
			      "All should be GetInfo commands");
	}
}

/* ========================================================================
 * End-to-End Performance Test
 * ======================================================================== */

/**
 * @brief Test complete command processing meets performance requirements
 *
 * Requirement: <100ms from command reception to mount response
 * Parser must contribute <10ms to this budget
 */
ZTEST(lx200, test_end_to_end_performance)
{
	ParserState parser;

	// Warm up
	for (const char c : std::string_view(":MS#")) {
		parser.feed_character(c);
	}
	parser.get_command();

	// Measure 100 command parse cycles
	uint32_t start = k_cycle_get_32();
	for (int i = 0; i < 100; i++) {
		parser.reset();
		for (const char c : std::string_view(":Sr12:34:56#")) {
			parser.feed_character(c);
		}
		auto cmd = parser.get_command();

		// Also parse the coordinate
		RACoordinate ra;
		parse_ra_coordinate(std::string(cmd->parameters).c_str(), PrecisionMode::High, ra);
	}
	uint32_t end = k_cycle_get_32();

	uint32_t cycles = end - start;
	uint32_t avg_cycles = cycles / 100;

	// At 168 MHz, 10ms = 1,680,000 cycles
	// Per command should be much less: <16,800 cycles
	zassert_true(avg_cycles < 20000, "End-to-end parsing should be fast (<10ms requirement)");
}

/* ========================================================================
 * Test Suite Registration
 * ======================================================================== */

extern "C" void test_suite_integration(void)
{
	// Tests are automatically registered via ZTEST macro
}

/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <lx200/lx200.hpp>

/**
 * @file test_commands.cpp
 * @brief LX200 Command Family Tests
 *
 * Tests command family identification for all 18 LX200 command families.
 * Based on data-model.md CommandFamily enum specification.
 */

using namespace lx200;

/* ========================================================================
 * Command Family Identification Tests
 * ======================================================================== */

/**
 * @brief Test Alignment command family (A)
 */
ZTEST(lx200, test_alignment_commands)
{
	ParserState parser;

	// :Aa# - Align to Alt/Az
	for (const char c : std::string_view(":Aa#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Aa command");
	zassert_equal(cmd->family, CommandFamily::Alignment, "Aa should be Alignment family");
}

/**
 * @brief Test Backup command family (B)
 */
ZTEST(lx200, test_backup_commands)
{
	ParserState parser;

	// :B+# - Increase backlash
	for (const char c : std::string_view(":B+#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse B+ command");
	zassert_equal(cmd->family, CommandFamily::Backup, "B+ should be Backup family");
}

/**
 * @brief Test DateTime command family (C, G, L, S for time/date)
 */
ZTEST(lx200, test_datetime_commands)
{
	ParserState parser;

	// :GC# - Get calendar date
	for (const char c : std::string_view(":GC#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GC command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "GC should be DateTime family");

	// :SC03/15/23# - Set calendar date
	parser.reset();
	for (const char c : std::string_view(":SC03/15/23#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse SC command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "SC should be DateTime family");
}

/**
 * @brief Test Distance command family (D)
 */
ZTEST(lx200, test_distance_commands)
{
	ParserState parser;

	// :D# - Distance bars
	for (const char c : std::string_view(":D#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse D command");
	zassert_equal(cmd->family, CommandFamily::Distance, "D should be Distance family");
}

/**
 * @brief Test Focus command family (F)
 */
ZTEST(lx200, test_focus_commands)
{
	ParserState parser;

	// :F+# - Focus inward
	for (const char c : std::string_view(":F+#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse F+ command");
	zassert_equal(cmd->family, CommandFamily::Focus, "F+ should be Focus family");
}

/**
 * @brief Test GetInfo command family (uppercase G)
 */
ZTEST(lx200, test_getinfo_commands)
{
	ParserState parser;

	// :GR# - Get RA
	for (const char c : std::string_view(":GR#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GR command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, "GR should be GetInfo family");

	// :GC# - Get calendar date
	parser.reset();
	for (const char c : std::string_view(":GC#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GC command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "GC should be DateTime family");

	// :GT# - Get tracking rate (uppercase G, not lowercase g)
	parser.reset();
	for (const char c : std::string_view(":GT#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GT command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, "GT should be GetInfo family");
}

/**
 * @brief Test GPS command family (lowercase g)
 * 
 * NOTE: According to LX200CommandSet.md Appendix B.2:
 * - :gT# is "Set Mount Time from GPS" (OAT/LX200GPS)
 * - This is a BLOCKING call that attempts GPS sync for 2 minutes
 * - Returns: 1 if data set, 0 if timeout
 * - NOT the same as :GT# (Get tracking rate)
 */
ZTEST(lx200, test_gps_commands)
{
	ParserState parser;

	// :gT# - Set mount time from GPS (lowercase g = GPS family)
	for (const char c : std::string_view(":gT#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse gT command");
	zassert_equal(cmd->family, CommandFamily::GPS, "gT should be GPS family");

	// :gTnnn# - Set mount time from GPS with timeout [OAT Extension]
	parser.reset();
	for (const char c : std::string_view(":gT5000#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse gTnnn command");
	zassert_equal(cmd->family, CommandFamily::GPS, "gTnnn should be GPS family");
	zassert_mem_equal(cmd->parameters.data(), "5000", 4, "Timeout parameter should be extracted");

	// :g+# - Turn on GPS power [LX200GPS]
	parser.reset();
	for (const char c : std::string_view(":g+#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse g+ command");
	zassert_equal(cmd->family, CommandFamily::GPS, "g+ should be GPS family");

	// :g-# - Turn off GPS power [LX200GPS]
	parser.reset();
	for (const char c : std::string_view(":g-#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse g- command");
	zassert_equal(cmd->family, CommandFamily::GPS, "g- should be GPS family");
}

/**
 * @brief Test Home command family (H)
 */
ZTEST(lx200, test_home_commands)
{
	ParserState parser;

	// :hP# - Park scope
	for (const char c : std::string_view(":hP#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse hP command");
	zassert_equal(cmd->family, CommandFamily::Home, "hP should be Home family");
}

/**
 * @brief Test Initialize command family (I)
 */
ZTEST(lx200, test_initialize_commands)
{
	ParserState parser;

	// :I# - Initialize
	for (const char c : std::string_view(":I#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse I command");
	zassert_equal(cmd->family, CommandFamily::Initialize, "I should be Initialize family");
}

/**
 * @brief Test Movement command family (M)
 */
ZTEST(lx200, test_movement_commands)
{
	ParserState parser;

	// :MS# - Slew to target
	for (const char c : std::string_view(":MS#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse MS command");
	zassert_equal(cmd->family, CommandFamily::Movement, "MS should be Movement family");

	// :Me# - Move east
	parser.reset();
	for (const char c : std::string_view(":Me#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Me command");
	zassert_equal(cmd->family, CommandFamily::Movement, "Me should be Movement family");
}

/**
 * @brief Test Precision command family (P)
 */
ZTEST(lx200, test_precision_commands)
{
	ParserState parser;

	// :P# - Toggle precision
	for (const char c : std::string_view(":P#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse P command");
	zassert_equal(cmd->family, CommandFamily::Precision, "P should be Precision family");
}

/**
 * @brief Test Quit command family (Q)
 */
ZTEST(lx200, test_quit_commands)
{
	ParserState parser;

	// :Q# - Halt movement
	for (const char c : std::string_view(":Q#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Q command");
	zassert_equal(cmd->family, CommandFamily::Quit, "Q should be Quit family");
}

/**
 * @brief Test Rate command family (R)
 */
ZTEST(lx200, test_rate_commands)
{
	ParserState parser;

	// :RS# - Slew rate
	for (const char c : std::string_view(":RS#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse RS command");
	zassert_equal(cmd->family, CommandFamily::Rate, "RS should be Rate family");
}

/**
 * @brief Test SetInfo command family (S)
 */
ZTEST(lx200, test_setinfo_commands)
{
	ParserState parser;

	// :Sr12:34:56# - Set target RA
	for (const char c : std::string_view(":Sr12:34:56#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Sr command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "Sr should be SetInfo family");
	zassert_mem_equal(cmd->parameters.data(), "12:34:56", 8, "Parameters should be extracted");
}

/**
 * @brief Test Tracking command family (T)
 */
ZTEST(lx200, test_tracking_commands)
{
	ParserState parser;

	// :T+# - Increment tracking rate
	for (const char c : std::string_view(":T+#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse T+ command");
	zassert_equal(cmd->family, CommandFamily::Tracking, "T+ should be Tracking family");
}

/**
 * @brief Test User command family (U)
 */
ZTEST(lx200, test_user_commands)
{
	ParserState parser;

	// :U# - User-defined
	for (const char c : std::string_view(":U#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse U command");
	zassert_equal(cmd->family, CommandFamily::User, "U should be User family");
}

/**
 * @brief Test Library command family (L)
 * 
 * NOTE: Library commands for object selection (Messier, NGC, etc.)
 * According to LX200CommandSet.md:
 * - :LMNNNN# - Select Messier object
 * - :LI# - Get object information
 * - :LB# - Find previous object
 * - :LN# - Find next object
 */
ZTEST(lx200, test_library_commands)
{
	ParserState parser;

	// :LI# - Get Object Information
	for (const char c : std::string_view(":LI#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse LI command");
	zassert_equal(cmd->family, CommandFamily::Library, "LI should be Library family");

	// :LMNNNN# - Set Messier object
	parser.reset();
	for (const char c : std::string_view(":LM0031#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse LM command");
	zassert_equal(cmd->family, CommandFamily::Library, "LM should be Library family");
	zassert_mem_equal(cmd->parameters.data(), "0031", 4, "Messier number should be extracted");
}

/**
 * @brief Test Extended OAT command family (X)
 * 
 * NOTE: According to LX200CommandSet.md Appendix B.9:
 * - 50+ OAT-specific commands starting with :X
 * - Used by OATControl PC application
 * - Examples: :XFR# (factory reset), :XGM# (get mount config),
 *   :XGB# (get backlash), :XSB# (set backlash), etc.
 */
ZTEST(lx200, test_extended_oat_commands)
{
	ParserState parser;

	// :XFR# - Factory Reset [OAT Extension]
	for (const char c : std::string_view(":XFR#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse XFR command");
	zassert_equal(cmd->family, CommandFamily::Extended, "XFR should be Extended family");
}

/**
 * @brief Test Unknown command family fallback
 */
ZTEST(lx200, test_unknown_command_family)
{
	ParserState parser;

	// :Z# - Not a standard LX200 command
	for (const char c : std::string_view(":Z#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Z command");
	zassert_equal(cmd->family, CommandFamily::Unknown, "Z should be Unknown family");
}

/* ========================================================================
 * Command Lookup Performance Test
 * ======================================================================== */

/**
 * @brief Test command family lookup is fast (compile-time table)
 */
ZTEST(lx200, test_command_lookup_performance)
{
	ParserState parser;

	// Warm up cache
	for (const char c : std::string_view(":GR#")) {
		parser.feed_character(c);
	}
	parser.get_command();

	// Measure lookup time
	uint32_t start = k_cycle_get_32();
	for (int i = 0; i < 100; i++) {
		parser.reset();
		for (const char c : std::string_view(":GR#")) {
			parser.feed_character(c);
		}
		parser.get_command();
	}
	uint32_t end = k_cycle_get_32();

	uint32_t cycles = end - start;
	uint32_t avg_cycles = cycles / 100;

	// Command lookup should be very fast (<1μs)
	// At 168 MHz, <1μs means <168 cycles
	zassert_true(avg_cycles < 500, "Command lookup should be fast (<1μs average)");
}

/* ========================================================================
 * Test Suite Registration
 * ======================================================================== */

extern "C" void test_suite_commands(void)
{
	// Tests are automatically registered via ZTEST macro
}

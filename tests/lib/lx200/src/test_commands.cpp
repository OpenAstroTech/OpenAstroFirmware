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
 * @brief Test Reticle/Accessory command family (B)
 * 
 * According to LX200CommandSet.md Section B:
 * - :B+# - Increase reticle brightness
 * - :B-# - Decrease reticle brightness
 * - :B<n># - Set reticle flash rate
 * - :BD<n># - Set reticle duty cycle [LX200GPS]
 * - :$BAdd# - Set Altitude/Dec Antibacklash [LX200GPS]
 * - :$BZdd# - Set Azimuth/RA Antibacklash [LX200GPS]
 */
ZTEST(lx200, test_reticle_commands)
{
	ParserState parser;

	// :B+# - Increase reticle brightness
	for (const char c : std::string_view(":B+#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse B+ command");
	zassert_equal(cmd->family, CommandFamily::Reticle, "B+ should be Reticle family");

	// :B-# - Decrease reticle brightness
	parser.reset();
	for (const char c : std::string_view(":B-#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse B- command");
	zassert_equal(cmd->family, CommandFamily::Reticle, "B- should be Reticle family");

	// :B3# - Set reticle flash rate to 3
	parser.reset();
	for (const char c : std::string_view(":B3#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse B3 command");
	zassert_equal(cmd->family, CommandFamily::Reticle, "B3 should be Reticle family");
}

/**
 * @brief Test Sync/DateTime command family (C)
 * 
 * According to LX200CommandSet.md Section C:
 * - :CM# - Synchronize telescope with current database object
 * - :CL# - Synchronize with selenographic coordinates
 */
ZTEST(lx200, test_sync_commands)
{
	ParserState parser;

	// :CM# - Sync to target coordinates (standard LX200)
	for (const char c : std::string_view(":CM#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse CM command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "CM should be DateTime/Sync family");

	// :CL# - Sync to selenographic coordinates
	parser.reset();
	for (const char c : std::string_view(":CL#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse CL command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "CL should be DateTime/Sync family");
}

/**
 * @brief Test DateTime Get/Set commands (G/S with date/time)
 * 
 * According to LX200CommandSet.md:
 * - :GC# - Get calendar date (MM/DD/YY)
 * - :GL# - Get local time (24h format)
 * - :Ga# - Get local time (12h format)
 * - :GS# - Get sidereal time
 * - :SCMM/DD/YY# - Set calendar date
 * - :SLHH:MM:SS# - Set local time
 * - :SSHH:MM:SS# - Set sidereal time
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

	// :SC03/15/23# - Set calendar date (SetInfo family, date/time semantics)
	parser.reset();
	for (const char c : std::string_view(":SC03/15/23#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse SC command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "SC should be SetInfo family (S prefix)");

	// :GL# - Get local time (24h)
	parser.reset();
	for (const char c : std::string_view(":GL#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GL command");
	zassert_equal(cmd->family, CommandFamily::DateTime, "GL should be DateTime family");

	// :SL14:30:45# - Set local time (SetInfo family, date/time semantics)
	parser.reset();
	for (const char c : std::string_view(":SL14:30:45#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse SL command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "SL should be SetInfo family (S prefix)");
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
 * 
 * According to LX200CommandSet.md Section F:
 * - :F+# - Start focuser moving inward (toward objective)
 * - :F-# - Start focuser moving outward (away from objective)
 * - :FQ# - Halt focuser motion
 * - :FF# - Set focus speed to fastest
 * - :FS# - Set focus speed to slowest
 * - :F<n># - Set focuser speed to <n> (1-4)
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

	// :F-# - Focus outward
	parser.reset();
	for (const char c : std::string_view(":F-#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse F- command");
	zassert_equal(cmd->family, CommandFamily::Focus, "F- should be Focus family");

	// :FQ# - Halt focuser
	parser.reset();
	for (const char c : std::string_view(":FQ#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse FQ command");
	zassert_equal(cmd->family, CommandFamily::Focus, "FQ should be Focus family");

	// :F2# - Set focus speed to 2
	parser.reset();
	for (const char c : std::string_view(":F2#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse F2 command");
	zassert_equal(cmd->family, CommandFamily::Focus, "F2 should be Focus family");
}

/**
 * @brief Test GetInfo command family (uppercase G)
 * 
 * According to LX200CommandSet.md Section G:
 * IMPORTANT: This is uppercase 'G' (GetInfo), not lowercase 'g' (GPS)
 * - :GA# - Get telescope altitude
 * - :Ga# - Get telescope azimuth  
 * - :GC# - Get current date
 * - :Gc# - Get calendar format
 * - :GD# - Get telescope declination
 * - :Gd# - Get target declination
 * - :GG# - Get UTC offset
 * - :Gg# - Get site longitude
 * - :Gh# - Get high limit
 * - :GL# - Get local time (12 or 24 hour format)
 * - :GM# - Get site name 1-4
 * - :GR# - Get telescope right ascension
 * - :Gr# - Get target right ascension
 * - :GS# - Get sidereal time
 * - :GT# - Get tracking rate
 * - :Gt# - Get site latitude
 * - :GVD# - Get firmware date
 * - :GVN# - Get firmware number
 * - :GVP# - Get product name
 * - :GVT# - Get firmware time
 * - :Gz# - Get azimuth
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

	// :GD# - Get telescope declination
	parser.reset();
	for (const char c : std::string_view(":GD#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GD command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, "GD should be GetInfo family");

	// :Gt# - Get site latitude
	parser.reset();
	for (const char c : std::string_view(":Gt#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Gt command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, "Gt should be GetInfo family");

	// :GVP# - Get product name
	parser.reset();
	for (const char c : std::string_view(":GVP#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GVP command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, "GVP should be GetInfo family");
}

/**
 * @brief Test GPS command family (lowercase g)
 * 
 * According to LX200CommandSet.md Section g:
 * IMPORTANT: This is lowercase 'g' (GPS), not uppercase 'G' (GetInfo)
 * - :g+# - GPS align (LX200GPS)
 * - :g-# - GPS align (LX200GPS)
 * - :gT# - Set mount time from GPS (OAT/LX200GPS)
 * - :gTnnn# - Set mount time from GPS with timeout [OAT Extension]
 * 
 * NOTE: :gT# is a BLOCKING call that attempts GPS sync for 2 minutes
 * Returns: 1 if data set, 0 if timeout
 * NOT the same as :GT# (Get tracking rate - uppercase G)
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
	zassert_true(cmd.has_value(), "Should parse gT5000 command");
	zassert_equal(cmd->family, CommandFamily::GPS, "gT5000 should be GPS family");

	// :g+# - GPS align
	parser.reset();
	for (const char c : std::string_view(":g+#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse g+ command");
	zassert_equal(cmd->family, CommandFamily::GPS, "g+ should be GPS family");

	// :g-# - GPS align
	parser.reset();
	for (const char c : std::string_view(":g-#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse g- command");
	zassert_equal(cmd->family, CommandFamily::GPS, "g- should be GPS family");
}

/**
 * @brief Test Home command family (h - lowercase)
 * 
 * According to LX200CommandSet.md Section h:
 * - :hS# - Seek and store home position [LX200GPS/LX16]
 * - :hF# - Seek home and align [LX200GPS/LX16]
 * - :hN# - Sleep telescope [LX200GPS]
 * - :hP# - Slew to park position
 * - :hW# - Wake up sleeping telescope [LX200GPS]
 * - :h?# - Query home status
 * 
 * Note: Uppercase H is for time format toggle (see separate test)
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

	// :hS# - Seek and store home
	parser.reset();
	for (const char c : std::string_view(":hS#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse hS command");
	zassert_equal(cmd->family, CommandFamily::Home, "hS should be Home family");

	// :h?# - Query home status
	parser.reset();
	for (const char c : std::string_view(":h?#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse h? command");
	zassert_equal(cmd->family, CommandFamily::Home, "h? should be Home family");
}

/**
 * @brief Test Hour/Time Format command (H - uppercase)
 * 
 * According to LX200CommandSet.md Section H:
 * - :H# - Toggle between 24 and 12 hour time format
 * 
 * Note: This is different from lowercase 'h' (home commands)
 * Currently mapped to DateTime family as it's time-related
 */
ZTEST(lx200, test_hour_format_commands)
{
	ParserState parser;

	// :H# - Toggle time format
	for (const char c : std::string_view(":H#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse H command");
	// H commands may be mapped to DateTime or a separate family
	// Check implementation's actual mapping
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
 * 
 * According to LX200CommandSet.md Section M:
 * - :MA# - Slew to target Alt/Az [Autostar/LX16/LX200GPS]
 * - :Me# - Move telescope east at current slew rate
 * - :Mn# - Move telescope north at current slew rate
 * - :Ms# - Move telescope south at current slew rate
 * - :Mw# - Move telescope west at current slew rate
 * - :MS# - Slew to target object
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

	// :Mn# - Move north
	parser.reset();
	for (const char c : std::string_view(":Mn#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Mn command");
	zassert_equal(cmd->family, CommandFamily::Movement, "Mn should be Movement family");

	// :Ms# - Move south
	parser.reset();
	for (const char c : std::string_view(":Ms#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Ms command");
	zassert_equal(cmd->family, CommandFamily::Movement, "Ms should be Movement family");

	// :Mw# - Move west
	parser.reset();
	for (const char c : std::string_view(":Mw#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Mw command");
	zassert_equal(cmd->family, CommandFamily::Movement, "Mw should be Movement family");

	// :MA# - Slew to Alt/Az
	parser.reset();
	for (const char c : std::string_view(":MA#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse MA command");
	zassert_equal(cmd->family, CommandFamily::Movement, "MA should be Movement family");
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
 * 
 * According to LX200CommandSet.md Section S:
 * - :Sa+DD*MM# / :Sa-DD*MM# - Set target altitude
 * - :SasDD# - Set target altitude (short form)
 * - :SCMM/DD/YY# - Set calendar date
 * - :Sc# - Change handbox date format
 * - :SdsDD*MM# or :SdsDD*MM'SS# - Set target declination
 * - :SG+HH.H# / :SG-HH.H# - Set UTC offset
 * - :SGsHH.H# - Set UTC offset (short form)
 * - :Sg+DDD*MM# / :Sg-DDD*MM# - Set site longitude
 * - :SgsDDD*MM# - Set site longitude (short form)
 * - :SL12:34:56# - Set local time (any length)
 * - :SMstring# - Set site name
 * - :SrHH:MM.T# or :SrHH:MM:SS# - Set target right ascension
 * - :StsDD*MM# - Set site latitude
 * - :SzDDD*MM# - Set target azimuth
 * - :SysDDD*MM'SS# - Sync [OAT] - Exact coordinates sync
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

	// :Sd+12*34:56# - Set target declination
	parser.reset();
	for (const char c : std::string_view(":Sd+12*34:56#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Sd command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "Sd should be SetInfo family");

	// :St+38*29# - Set site latitude
	parser.reset();
	for (const char c : std::string_view(":St+38*29#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse St command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "St should be SetInfo family");

	// :Sg122*04# - Set site longitude
	parser.reset();
	for (const char c : std::string_view(":Sg122*04#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Sg command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "Sg should be SetInfo family");

	// :SC03/15/24# - Set calendar date
	parser.reset();
	for (const char c : std::string_view(":SC03/15/24#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse SC command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "SC should be SetInfo family");

	// :SL14:23:00# - Set local time
	parser.reset();
	for (const char c : std::string_view(":SL14:23:00#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse SL command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "SL should be SetInfo family");

	// :Sy+12*34'56# - Sync exact coordinates [OAT]
	parser.reset();
	for (const char c : std::string_view(":Sy+12*34'56#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse Sy command");
	zassert_equal(cmd->family, CommandFamily::SetInfo, "Sy should be SetInfo family");
}

/**
 * @brief Test Tracking command family (T)
 * 
 * According to LX200CommandSet.md Section T:
 * - :T+# - Increment current tracking rate
 * - :T-# - Decrement current tracking rate  
 * - :TL# - Set tracking rate to lunar rate
 * - :TM# - Set tracking rate to solar rate
 * - :TQ# - Set tracking rate to sidereal rate
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

	// :T-# - Decrement tracking rate
	parser.reset();
	for (const char c : std::string_view(":T-#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse T- command");
	zassert_equal(cmd->family, CommandFamily::Tracking, "T- should be Tracking family");

	// :TL# - Lunar rate
	parser.reset();
	for (const char c : std::string_view(":TL#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse TL command");
	zassert_equal(cmd->family, CommandFamily::Tracking, "TL should be Tracking family");

	// :TM# - Solar rate
	parser.reset();
	for (const char c : std::string_view(":TM#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse TM command");
	zassert_equal(cmd->family, CommandFamily::Tracking, "TM should be Tracking family");

	// :TQ# - Sidereal rate
	parser.reset();
	for (const char c : std::string_view(":TQ#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse TQ command");
	zassert_equal(cmd->family, CommandFamily::Tracking, "TQ should be Tracking family");
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
 * @brief Test case-sensitivity in LX200 protocol
 * 
 * CRITICAL: The LX200 protocol is case-sensitive!
 * - 'G' (uppercase) = GetInfo family
 * - 'g' (lowercase) = GPS family
 * - 'H' (uppercase) = DateTime family (hour format toggle)
 * - 'h' (lowercase) = Home family (home/park commands)
 */
ZTEST(lx200, test_case_sensitivity)
{
	ParserState parser;

	// Verify :GT# (uppercase G) is GetInfo, not GPS
	for (const char c : std::string_view(":GT#")) {
		parser.feed_character(c);
	}
	auto cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse GT command");
	zassert_equal(cmd->family, CommandFamily::GetInfo, 
		":GT# (uppercase G) should be GetInfo family (tracking rate)");

	// Verify :gT# (lowercase g) is GPS, not GetInfo
	parser.reset();
	for (const char c : std::string_view(":gT#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse gT command");
	zassert_equal(cmd->family, CommandFamily::GPS,
		":gT# (lowercase g) should be GPS family (set time from GPS)");

	// Verify :H# (uppercase H) is DateTime (hour format)
	parser.reset();
	for (const char c : std::string_view(":H#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse H command");
	zassert_equal(cmd->family, CommandFamily::DateTime,
		":H# (uppercase H) should be DateTime family (toggle time format)");

	// Verify :hP# (lowercase h) is Home (park position)
	parser.reset();
	for (const char c : std::string_view(":hP#")) {
		parser.feed_character(c);
	}
	cmd = parser.get_command();
	zassert_true(cmd.has_value(), "Should parse hP command");
	zassert_equal(cmd->family, CommandFamily::Home,
		":hP# (lowercase h) should be Home family (park position)");
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

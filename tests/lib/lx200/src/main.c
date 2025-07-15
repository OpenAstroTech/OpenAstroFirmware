/**
 * @file main.c
 * @brief LX200 Library Test Suite
 *
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/ztest.h>
#include <lx200/lx200.h>

/* Test helper macros */
#define ASSERT_PARSE_OK(result) zassert_equal(result, LX200_PARSE_OK, "Parse should succeed")
#define ASSERT_PARSE_ERROR(result, expected) zassert_equal(result, expected, "Expected specific parse error")

/* Test fixtures */
static lx200_parser_state_t parser_state;
static lx200_command_t command;

/**
 * @brief Setup function called before each test
 */
static void lx200_test_setup(void *fixture)
{
	ARG_UNUSED(fixture);
	lx200_parser_init(&parser_state);
	memset(&command, 0, sizeof(command));
}

/**
 * @brief Teardown function called after each test
 */
static void lx200_test_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	lx200_parser_reset(&parser_state);
}

/* ============================================================================
 * PARSER INITIALIZATION AND STATE TESTS
 * ============================================================================ */

ZTEST(lx200_parser, test_parser_init_valid)
{
	lx200_parser_state_t state;
	
	lx200_parser_init(&state);
	
	zassert_equal(state.buffer_length, 0, "Buffer length should be 0");
	zassert_false(state.command_complete, "Command should not be complete");
	zassert_equal(state.precision_mode, LX200_COORD_HIGH_PRECISION, 
		      "Should default to high precision");
	zassert_equal(state.buffer[0], '\0', "Buffer should be null-terminated");
}

ZTEST(lx200_parser, test_parser_init_null)
{
	/* Should not crash with NULL pointer */
	lx200_parser_init(NULL);
	zassert_true(true, "Should handle NULL gracefully");
}

ZTEST(lx200_parser, test_parser_reset)
{
	/* Setup some state first */
	strcpy(parser_state.buffer, ":Gr");
	parser_state.buffer_length = 3;
	parser_state.command_complete = true;
	
	lx200_parser_reset(&parser_state);
	
	zassert_equal(parser_state.buffer_length, 0, "Buffer length should be 0");
	zassert_false(parser_state.command_complete, "Command should not be complete");
	zassert_equal(parser_state.buffer[0], '\0', "Buffer should be null-terminated");
}

ZTEST(lx200_parser, test_precision_mode_functions)
{
	/* Test getting current precision */
	lx200_precision_t precision = lx200_get_precision_mode(&parser_state);
	zassert_equal(precision, LX200_COORD_HIGH_PRECISION, "Should be high precision");
	
	/* Test setting precision */
	lx200_set_precision_mode(&parser_state, LX200_COORD_LOW_PRECISION);
	precision = lx200_get_precision_mode(&parser_state);
	zassert_equal(precision, LX200_COORD_LOW_PRECISION, "Should be low precision");
	
	/* Test NULL handling */
	precision = lx200_get_precision_mode(NULL);
	zassert_equal(precision, LX200_COORD_HIGH_PRECISION, "Should default to high precision");
	
	lx200_set_precision_mode(NULL, LX200_COORD_LOW_PRECISION);
	/* Should not crash */
	zassert_true(true, "Should handle NULL gracefully");
}

/* ============================================================================
 * COMMAND PARSING TESTS
 * ============================================================================ */

ZTEST(lx200_parser, test_add_data_valid_complete_command)
{
	const char *cmd = ":Gr#";
	lx200_parse_result_t result;
	
	result = lx200_parser_add_data(&parser_state, cmd, strlen(cmd));
	
	ASSERT_PARSE_OK(result);
	zassert_true(parser_state.command_complete, "Command should be complete");
	zassert_equal(parser_state.buffer_length, strlen(cmd), "Buffer length should match");
	zassert_mem_equal(parser_state.buffer, cmd, strlen(cmd), "Buffer should contain command");
}

ZTEST(lx200_parser, test_add_data_incomplete_command)
{
	const char *cmd = ":Gr";
	lx200_parse_result_t result;
	
	result = lx200_parser_add_data(&parser_state, cmd, strlen(cmd));
	
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
	zassert_false(parser_state.command_complete, "Command should not be complete");
	zassert_equal(parser_state.buffer_length, strlen(cmd), "Buffer length should match");
}

ZTEST(lx200_parser, test_add_data_incremental)
{
	lx200_parse_result_t result;
	
	/* Add data incrementally */
	result = lx200_parser_add_data(&parser_state, ":", 1);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
	
	result = lx200_parser_add_data(&parser_state, "G", 1);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
	
	result = lx200_parser_add_data(&parser_state, "r", 1);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
	
	result = lx200_parser_add_data(&parser_state, "#", 1);
	ASSERT_PARSE_OK(result);
	
	zassert_true(parser_state.command_complete, "Command should be complete");
	zassert_str_equal(parser_state.buffer, ":Gr#", "Buffer should contain complete command");
}

ZTEST(lx200_parser, test_add_data_buffer_overflow)
{
	char long_data[LX200_MAX_COMMAND_LENGTH + 10];
	lx200_parse_result_t result;
	
	memset(long_data, 'X', sizeof(long_data) - 1);
	long_data[sizeof(long_data) - 1] = '\0';
	
	result = lx200_parser_add_data(&parser_state, long_data, sizeof(long_data) - 1);
	
	ASSERT_PARSE_ERROR(result, LX200_PARSE_BUFFER_OVERFLOW);
}

ZTEST(lx200_parser, test_add_data_invalid_parameters)
{
	lx200_parse_result_t result;
	
	/* NULL state */
	result = lx200_parser_add_data(NULL, ":Gr#", 4);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_ERROR);
	
	/* NULL data */
	result = lx200_parser_add_data(&parser_state, NULL, 4);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_ERROR);
	
	/* Zero length */
	result = lx200_parser_add_data(&parser_state, ":Gr#", 0);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_ERROR);
}

ZTEST(lx200_parser, test_parse_command_string_get_ra)
{
	const char *cmd = ":Gr#";
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Gr", "Command should be 'Gr'");
	zassert_equal(command.family, LX200_CMD_GET, "Should be GET command family");
	zassert_false(command.has_parameter, "Should not have parameter");
	zassert_equal(command.parameter_length, 0, "Parameter length should be 0");
}

ZTEST(lx200_parser, test_parse_command_string_set_ra_with_parameter)
{
	const char *cmd = ":Sr14:30:45#";
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Sr", "Command should be 'Sr'");
	zassert_equal(command.family, LX200_CMD_SET, "Should be SET command family");
	zassert_true(command.has_parameter, "Should have parameter");
	zassert_str_equal(command.parameter, "14:30:45", "Parameter should be time");
	zassert_equal(command.parameter_length, 8, "Parameter length should be 8");
}

ZTEST(lx200_parser, test_parse_command_string_various_commands)
{
	struct {
		const char *cmd;
		const char *expected_command;
		lx200_command_family_t expected_family;
		bool has_param;
	} test_cases[] = {
		{":AA#", "AA", LX200_CMD_ALIGNMENT, false},
		{":B+#", "B+", LX200_CMD_RETICLE, false},
		{":CM#", "CM", LX200_CMD_SYNC, false},
		{":D#", "D", LX200_CMD_DISTANCE, false},
		{":F+#", "F+", LX200_CMD_FOCUSER, false},
		{":GD#", "GD", LX200_CMD_GET, false},
		{":gT#", "gT", LX200_CMD_GPS, false},
		{":H#", "H", LX200_CMD_TIME_FORMAT, false},
		{":I#", "I", LX200_CMD_INITIALIZE, false},
		{":LM#", "LM", LX200_CMD_LIBRARY, false},
		{":Mn#", "Mn", LX200_CMD_MOVE, false},
		{":P#", "P", LX200_CMD_PRECISION, false},
		{":Q#", "Q", LX200_CMD_STOP, false},
		{":RG#", "RG", LX200_CMD_SLEW_RATE, false},
		{":Sd+45*30:15#", "Sd", LX200_CMD_SET, true},
		{":TL#", "TL", LX200_CMD_TRACKING, false},
		{":U#", "U", LX200_CMD_PRECISION_TOGGLE, false},
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		memset(&command, 0, sizeof(command));
		lx200_parse_result_t result = lx200_parse_command_string(test_cases[i].cmd, &command);
		
		ASSERT_PARSE_OK(result);
		zassert_str_equal(command.command, test_cases[i].expected_command,
				  "Command mismatch for %s", test_cases[i].cmd);
		zassert_equal(command.family, test_cases[i].expected_family,
			      "Family mismatch for %s", test_cases[i].cmd);
		zassert_equal(command.has_parameter, test_cases[i].has_param,
			      "Parameter flag mismatch for %s", test_cases[i].cmd);
	}
}

ZTEST(lx200_parser, test_parse_command_string_invalid_prefix)
{
	const char *cmd = "Gr#";  /* Missing ':' prefix */
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INVALID_PREFIX);
}

ZTEST(lx200_parser, test_parse_command_string_invalid_terminator)
{
	const char *cmd = ":Gr";  /* Missing '#' terminator */
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INVALID_TERMINATOR);
}

ZTEST(lx200_parser, test_parse_command_string_too_short)
{
	const char *cmd = ":";  /* Too short */
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INVALID_COMMAND);
}

ZTEST(lx200_parser, test_parse_command_string_null_parameters)
{
	lx200_parse_result_t result;
	
	/* NULL command string */
	result = lx200_parse_command_string(NULL, &command);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_ERROR);
	
	/* NULL command structure */
	result = lx200_parse_command_string(":Gr#", NULL);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_ERROR);
}

ZTEST(lx200_parser, test_parse_command_from_state)
{
	lx200_parse_result_t result;
	
	/* Add complete command to parser */
	result = lx200_parser_add_data(&parser_state, ":Gr#", 4);
	ASSERT_PARSE_OK(result);
	
	/* Parse command from state */
	result = lx200_parse_command(&parser_state, &command);
	ASSERT_PARSE_OK(result);
	
	zassert_str_equal(command.command, "Gr", "Command should be 'Gr'");
	zassert_equal(command.family, LX200_CMD_GET, "Should be GET command family");
}

ZTEST(lx200_parser, test_parse_command_incomplete_state)
{
	lx200_parse_result_t result;
	
	/* Add incomplete command to parser */
	result = lx200_parser_add_data(&parser_state, ":Gr", 3);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
	
	/* Try to parse incomplete command */
	result = lx200_parse_command(&parser_state, &command);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_INCOMPLETE);
}

/* ============================================================================
 * COMMAND FAMILY AND UTILITY TESTS
 * ============================================================================ */

ZTEST(lx200_utils, test_get_command_family)
{
	struct {
		const char *cmd;
		lx200_command_family_t expected;
	} test_cases[] = {
		{"A", LX200_CMD_ALIGNMENT},
		{"AA", LX200_CMD_ALIGNMENT},
		{"B+", LX200_CMD_RETICLE},
		{"C", LX200_CMD_SYNC},
		{"D", LX200_CMD_DISTANCE},
		{"F", LX200_CMD_FOCUSER},
		{"G", LX200_CMD_GET},
		{"Gr", LX200_CMD_GET},
		{"g", LX200_CMD_GPS},
		{"gT", LX200_CMD_GPS},
		{"H", LX200_CMD_TIME_FORMAT},
		{"I", LX200_CMD_INITIALIZE},
		{"L", LX200_CMD_LIBRARY},
		{"M", LX200_CMD_MOVE},
		{"P", LX200_CMD_PRECISION},
		{"Q", LX200_CMD_STOP},
		{"R", LX200_CMD_SLEW_RATE},
		{"S", LX200_CMD_SET},
		{"T", LX200_CMD_TRACKING},
		{"U", LX200_CMD_PRECISION_TOGGLE},
		{"X", LX200_CMD_UNKNOWN},
		{"", LX200_CMD_UNKNOWN},
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		lx200_command_family_t family = lx200_get_command_family(test_cases[i].cmd);
		zassert_equal(family, test_cases[i].expected,
			      "Family mismatch for command '%s'", test_cases[i].cmd);
	}
	
	/* Test NULL parameter */
	lx200_command_family_t family = lx200_get_command_family(NULL);
	zassert_equal(family, LX200_CMD_UNKNOWN, "Should return UNKNOWN for NULL");
}

ZTEST(lx200_utils, test_command_has_parameter)
{
	struct {
		const char *cmd;
		bool expected;
	} test_cases[] = {
		{"Sr", true},   /* Set commands typically have parameters */
		{"Sd", true},
		{"SC", true},
		{"Gr", false},  /* Get commands typically don't */
		{"AA", false},  /* Alignment commands don't */
		{"Q", false},   /* Stop commands don't */
		{"", false},    /* Empty command */
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		bool has_param = lx200_command_has_parameter(test_cases[i].cmd);
		zassert_equal(has_param, test_cases[i].expected,
			      "Parameter flag mismatch for command '%s'", test_cases[i].cmd);
	}
	
	/* Test NULL parameter */
	bool has_param = lx200_command_has_parameter(NULL);
	zassert_false(has_param, "Should return false for NULL");
}

ZTEST(lx200_utils, test_get_parameter_format)
{
	struct {
		const char *cmd;
		const char *expected;
	} test_cases[] = {
		{"Sr", "HH:MM:SS"},
		{"Sd", "sDD*MM:SS"},
		{"SL", "HH:MM:SS"},
		{"SC", "MM/DD/YY"},
		{"S", "Various"},
		{"G", "None"},
		{"", "None"},
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		const char *format = lx200_get_parameter_format(test_cases[i].cmd);
		zassert_str_equal(format, test_cases[i].expected,
				  "Format mismatch for command '%s'", test_cases[i].cmd);
	}
	
	/* Test NULL parameter */
	const char *format = lx200_get_parameter_format(NULL);
	zassert_str_equal(format, "None", "Should return 'None' for NULL");
}

ZTEST(lx200_utils, test_parse_result_to_string)
{
	struct {
		lx200_parse_result_t result;
		const char *expected;
	} test_cases[] = {
		{LX200_PARSE_OK, "OK"},
		{LX200_PARSE_INCOMPLETE, "Incomplete"},
		{LX200_PARSE_INVALID_PREFIX, "Invalid prefix"},
		{LX200_PARSE_INVALID_TERMINATOR, "Invalid terminator"},
		{LX200_PARSE_INVALID_COMMAND, "Invalid command"},
		{LX200_PARSE_INVALID_PARAMETER, "Invalid parameter"},
		{LX200_PARSE_BUFFER_OVERFLOW, "Buffer overflow"},
		{LX200_PARSE_ERROR, "Parse error"},
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		const char *str = lx200_parse_result_to_string(test_cases[i].result);
		zassert_str_equal(str, test_cases[i].expected,
				  "String mismatch for result %d", test_cases[i].result);
	}
	
	/* Test invalid result code */
	const char *str = lx200_parse_result_to_string((lx200_parse_result_t)999);
	zassert_str_equal(str, "Parse error", "Should return 'Parse error' for invalid code");
}

/* ============================================================================
 * COMPLEX COMMAND PARSING TESTS
 * ============================================================================ */

ZTEST(lx200_complex, test_long_parameter_command)
{
	const char *cmd = ":Sr12:34:56#";
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Sr", "Command should be 'Sr'");
	zassert_true(command.has_parameter, "Should have parameter");
	zassert_str_equal(command.parameter, "12:34:56", "Parameter should be time");
	zassert_equal(command.parameter_length, 8, "Parameter length should be 8");
}

ZTEST(lx200_complex, test_multiple_commands_sequential)
{
	lx200_parse_result_t result;
	
	/* First command */
	result = lx200_parser_add_data(&parser_state, ":Gr#", 4);
	ASSERT_PARSE_OK(result);
	
	result = lx200_parse_command(&parser_state, &command);
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Gr", "First command should be 'Gr'");
	
	/* Reset and parse second command */
	lx200_parser_reset(&parser_state);
	
	result = lx200_parser_add_data(&parser_state, ":Gd#", 4);
	ASSERT_PARSE_OK(result);
	
	result = lx200_parse_command(&parser_state, &command);
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Gd", "Second command should be 'Gd'");
}

ZTEST(lx200_complex, test_command_with_special_characters)
{
	const char *cmd = ":Sd+45*30:15#";
	lx200_parse_result_t result;
	
	result = lx200_parse_command_string(cmd, &command);
	
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Sd", "Command should be 'Sd'");
	zassert_true(command.has_parameter, "Should have parameter");
	zassert_str_equal(command.parameter, "+45*30:15", "Parameter should contain special chars");
}

ZTEST(lx200_complex, test_boundary_conditions)
{
	lx200_parse_result_t result;
	char buffer[LX200_MAX_COMMAND_LENGTH];
	
	/* Test maximum length command */
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = ':';
	buffer[1] = 'S';
	buffer[2] = 'r';
	/* Fill with parameter data up to max length - 2 (for ':' and '#') */
	for (int i = 3; i < LX200_MAX_COMMAND_LENGTH - 1; i++) {
		buffer[i] = '0' + (i % 10);
	}
	buffer[LX200_MAX_COMMAND_LENGTH - 1] = '#';
	
	result = lx200_parse_command_string(buffer, &command);
	ASSERT_PARSE_OK(result);
	zassert_str_equal(command.command, "Sr", "Command should be parsed correctly");
	zassert_true(command.has_parameter, "Should have parameter");
}

/* ============================================================================
 * ERROR HANDLING TESTS
 * ============================================================================ */

ZTEST(lx200_errors, test_malformed_commands)
{
	struct {
		const char *cmd;
		lx200_parse_result_t expected;
	} test_cases[] = {
		{"", LX200_PARSE_INVALID_COMMAND},
		{"#", LX200_PARSE_INVALID_COMMAND},
		{":", LX200_PARSE_INVALID_COMMAND},
		{":G", LX200_PARSE_INVALID_TERMINATOR},
		{"G#", LX200_PARSE_INVALID_PREFIX},
		{":G$", LX200_PARSE_INVALID_TERMINATOR},
		{"$G#", LX200_PARSE_INVALID_PREFIX},
	};
	
	for (size_t i = 0; i < ARRAY_SIZE(test_cases); i++) {
		lx200_parse_result_t result = lx200_parse_command_string(test_cases[i].cmd, &command);
		zassert_equal(result, test_cases[i].expected,
			      "Error mismatch for command '%s'", test_cases[i].cmd);
	}
}

ZTEST(lx200_errors, test_parameter_too_long)
{
	char long_cmd[LX200_MAX_COMMAND_LENGTH * 2];
	lx200_parse_result_t result;
	
	/* Create command with parameter that's too long */
	strcpy(long_cmd, ":Sr");
	for (int i = 3; i < sizeof(long_cmd) - 2; i++) {
		long_cmd[i] = '0';
	}
	long_cmd[sizeof(long_cmd) - 2] = '#';
	long_cmd[sizeof(long_cmd) - 1] = '\0';
	
	result = lx200_parse_command_string(long_cmd, &command);
	ASSERT_PARSE_ERROR(result, LX200_PARSE_BUFFER_OVERFLOW);
}

/* ============================================================================
 * TEST SUITE DEFINITIONS
 * ============================================================================ */

ZTEST_SUITE(lx200_parser, NULL, NULL, lx200_test_setup, lx200_test_teardown, NULL);
ZTEST_SUITE(lx200_utils, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(lx200_complex, NULL, NULL, lx200_test_setup, lx200_test_teardown, NULL);
ZTEST_SUITE(lx200_errors, NULL, NULL, NULL, NULL, NULL);

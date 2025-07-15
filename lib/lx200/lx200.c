/**
 * @file lx200.c
 * @brief LX200 Protocol Parser Implementation
 *
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lx200/lx200.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lx200, CONFIG_LX200_LOG_LEVEL);

/**
 * @brief Initialize LX200 parser state
 */
void lx200_parser_init(lx200_parser_state_t *state)
{
	if (state == NULL) {
		LOG_ERR("lx200_parser_init: NULL state pointer");
		return;
	}

	memset(state->buffer, 0, sizeof(state->buffer));
	state->buffer_length = 0;
	state->command_complete = false;
	state->precision_mode = LX200_COORD_HIGH_PRECISION;

	LOG_DBG("LX200 parser initialized with high precision mode");
}

/**
 * @brief Reset LX200 parser state
 */
void lx200_parser_reset(lx200_parser_state_t *state)
{
	if (state == NULL) {
		LOG_ERR("lx200_parser_reset: NULL state pointer");
		return;
	}

	LOG_DBG("Resetting LX200 parser state (buffer_length=%zu)", state->buffer_length);

	memset(state->buffer, 0, sizeof(state->buffer));
	state->buffer_length = 0;
	state->command_complete = false;
}

/**
 * @brief Convert parse result to string
 */
const char *lx200_parse_result_to_string(lx200_parse_result_t result)
{
	switch (result) {
	case LX200_PARSE_OK:
		return "OK";
	case LX200_PARSE_INCOMPLETE:
		return "Incomplete";
	case LX200_PARSE_INVALID_PREFIX:
		return "Invalid prefix";
	case LX200_PARSE_INVALID_TERMINATOR:
		return "Invalid terminator";
	case LX200_PARSE_INVALID_COMMAND:
		return "Invalid command";
	case LX200_PARSE_INVALID_PARAMETER:
		return "Invalid parameter";
	case LX200_PARSE_BUFFER_OVERFLOW:
		return "Buffer overflow";
	case LX200_PARSE_ERROR:
	default:
		return "Parse error";
	}
}

/**
 * @brief Set precision mode
 */
void lx200_set_precision_mode(lx200_parser_state_t *state, lx200_precision_t precision)
{
	if (state == NULL) {
		LOG_ERR("lx200_set_precision_mode: NULL state pointer");
		return;
	}

	LOG_INF("Changing precision mode from %d to %d", state->precision_mode, precision);
	state->precision_mode = precision;
}

/**
 * @brief Get current precision mode
 */
lx200_precision_t lx200_get_precision_mode(const lx200_parser_state_t *state)
{
	if (state == NULL) {
		return LX200_COORD_HIGH_PRECISION;
	}

	return state->precision_mode;
}

/**
 * @brief Get command family from command string
 */
lx200_command_family_t lx200_get_command_family(const char *command)
{
	if (command == NULL || command[0] == '\0') {
		LOG_ERR("lx200_get_command_family: Invalid command string");
		return LX200_CMD_UNKNOWN;
	}

	lx200_command_family_t family;

	switch (command[0]) {
	case 'A':
		family = LX200_CMD_ALIGNMENT;
		break;
	case 'B':
		family = LX200_CMD_RETICLE;
		break;
	case 'C':
		family = LX200_CMD_SYNC;
		break;
	case 'D':
		family = LX200_CMD_DISTANCE;
		break;
	case 'F':
		family = LX200_CMD_FOCUSER;
		break;
	case 'G':
		family = LX200_CMD_GET;
		break;
	case 'g':
		family = LX200_CMD_GPS;
		break;
	case 'H':
		family = LX200_CMD_TIME_FORMAT;
		break;
	case 'I':
		family = LX200_CMD_INITIALIZE;
		break;
	case 'L':
		family = LX200_CMD_LIBRARY;
		break;
	case 'M':
		family = LX200_CMD_MOVE;
		break;
	case 'P':
		family = LX200_CMD_PRECISION;
		break;
	case 'Q':
		family = LX200_CMD_STOP;
		break;
	case 'R':
		family = LX200_CMD_SLEW_RATE;
		break;
	case 'S':
		family = LX200_CMD_SET;
		break;
	case 'T':
		family = LX200_CMD_TRACKING;
		break;
	case 'U':
		family = LX200_CMD_PRECISION_TOGGLE;
		break;
	default:
		family = LX200_CMD_UNKNOWN;
		LOG_WRN("Unknown command family for command '%s' (first char: '%c')", command,
			command[0]);
		break;
	}

	if (family != LX200_CMD_UNKNOWN) {
		LOG_DBG("Command '%s' mapped to family %d", command, family);
	}

	return family;
}

/**
 * @brief Check if command expects a parameter
 */
bool lx200_command_has_parameter(const char *command)
{
	if (command == NULL || command[0] == '\0') {
		return false;
	}

	// Commands that typically have parameters start with 'S' (Set commands)
	// This is a simplified implementation - in practice, you'd need a lookup table
	return (command[0] == 'S');
}

/**
 * @brief Get expected parameter format for command
 */
const char *lx200_get_parameter_format(const char *command)
{
	if (command == NULL || command[0] == '\0') {
		return "None";
	}

	// This is a simplified implementation
	// In practice, you'd need a comprehensive lookup table
	switch (command[0]) {
	case 'S':
		if (command[1] == 'r') {
			return "HH:MM:SS";
		}
		if (command[1] == 'd') {
			return "sDD*MM:SS";
		}
		if (command[1] == 'L') {
			return "HH:MM:SS";
		}
		if (command[1] == 'C') {
			return "MM/DD/YY";
		}
		return "Various";
	default:
		return "None";
	}
}

/**
 * @brief Add data to LX200 parser buffer
 */
lx200_parse_result_t lx200_parser_add_data(lx200_parser_state_t *state, const char *data,
					   size_t length)
{
	if (state == NULL || data == NULL || length == 0) {
		LOG_ERR("lx200_parser_add_data: Invalid parameters (state=%p, data=%p, length=%zu)",
			state, data, length);
		return LX200_PARSE_ERROR;
	}

	LOG_DBG("Adding %zu bytes to parser buffer (current length: %zu)", length,
		state->buffer_length);

	// Check for buffer overflow
	if (state->buffer_length + length >= LX200_MAX_COMMAND_LENGTH) {
		LOG_ERR("Buffer overflow: current=%zu, adding=%zu, max=%d", state->buffer_length,
			length, LX200_MAX_COMMAND_LENGTH);
		return LX200_PARSE_BUFFER_OVERFLOW;
	}

	// Copy data to buffer
	memcpy(&state->buffer[state->buffer_length], data, length);
	state->buffer_length += length;
	state->buffer[state->buffer_length] = '\0';

	LOG_HEXDUMP_DBG(data, length, "Received data:");

	// Check if command is complete (ends with #)
	if (state->buffer_length > 0 &&
	    state->buffer[state->buffer_length - 1] == LX200_COMMAND_TERMINATOR) {
		state->command_complete = true;
		LOG_DBG("Command complete: '%s'", state->buffer);
		return LX200_PARSE_OK;
	}

	LOG_DBG("Command incomplete, buffer: '%s'", state->buffer);
	return LX200_PARSE_INCOMPLETE;
}

/**
 * @brief Parse complete LX200 command
 */
lx200_parse_result_t lx200_parse_command(const lx200_parser_state_t *state,
					 lx200_command_t *command)
{
	if (state == NULL || command == NULL) {
		LOG_ERR("lx200_parse_command: Invalid parameters (state=%p, command=%p)", state,
			command);
		return LX200_PARSE_ERROR;
	}

	if (!state->command_complete) {
		LOG_DBG("Command not complete yet, current buffer: '%s'", state->buffer);
		return LX200_PARSE_INCOMPLETE;
	}

	LOG_DBG("Parsing complete command: '%s'", state->buffer);
	return lx200_parse_command_string(state->buffer, command);
}

/**
 * @brief Parse LX200 command from string
 */
lx200_parse_result_t lx200_parse_command_string(const char *cmd_string, lx200_command_t *command)
{
	if (cmd_string == NULL || command == NULL) {
		LOG_ERR("lx200_parse_command_string: Invalid parameters (cmd_string=%p, "
			"command=%p)",
			cmd_string, command);
		return LX200_PARSE_ERROR;
	}

	size_t len = strlen(cmd_string);
	LOG_DBG("Parsing command string: '%s' (length: %zu)", cmd_string, len);

	if (len < 2) {
		LOG_ERR("Command too short: %zu bytes", len);
		return LX200_PARSE_INVALID_COMMAND;
	}

	// Check for valid prefix
	if (cmd_string[0] != LX200_COMMAND_PREFIX) {
		LOG_ERR("Invalid command prefix: expected '%c', got '%c'", LX200_COMMAND_PREFIX,
			cmd_string[0]);
		return LX200_PARSE_INVALID_PREFIX;
	}

	// Check for valid terminator
	if (cmd_string[len - 1] != LX200_COMMAND_TERMINATOR) {
		LOG_ERR("Invalid command terminator: expected '%c', got '%c'",
			LX200_COMMAND_TERMINATOR, cmd_string[len - 1]);
		return LX200_PARSE_INVALID_TERMINATOR;
	}

	// Extract command (skip prefix, stop at terminator or parameter start)
	size_t cmd_len = 0;
	for (size_t i = 1; i < len - 1 && cmd_len < 3; i++) {
		char c = cmd_string[i];
		// Include alphabetic characters
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			command->command[cmd_len++] = c;
		} else if (cmd_len > 0) {
			// Check if this might be a symbol that's part of the command
			// Only for specific command families that use symbol suffixes
			char first_char = command->command[0];
			if ((first_char == 'B' || first_char == 'F' || first_char == 'T') && 
			    (c == '+' || c == '-')) {
				// These commands use +/- as part of the command
				command->command[cmd_len++] = c;
			} else if (first_char == 'R' && (c >= '0' && c <= '9')) {
				// Rate commands can have numbers
				command->command[cmd_len++] = c;
			} else {
				// Stop here - this starts the parameter
				break;
			}
		} else {
			// Stop at first character that's not part of a command
			break;
		}
	}
	command->command[cmd_len] = '\0';

	LOG_DBG("Extracted command: '%s'", command->command);

	// Get command family
	command->family = lx200_get_command_family(command->command);
	LOG_DBG("Command family: %d", command->family);

	// Extract parameter if present
	size_t param_start = 1 + cmd_len;
	if (param_start < len - 1) {
		command->has_parameter = true;
		command->parameter_length = len - 1 - param_start;
		if (command->parameter_length < LX200_MAX_COMMAND_LENGTH) {
			memcpy(command->parameter, &cmd_string[param_start],
			       command->parameter_length);
			command->parameter[command->parameter_length] = '\0';
			LOG_DBG("Extracted parameter: '%s' (length: %zu)", command->parameter,
				command->parameter_length);
		} else {
			LOG_ERR("Parameter too long: %zu bytes", command->parameter_length);
			return LX200_PARSE_BUFFER_OVERFLOW;
		}
	} else {
		command->has_parameter = false;
		command->parameter_length = 0;
		command->parameter[0] = '\0';
		LOG_DBG("No parameter present");
	}

	LOG_INF("Successfully parsed LX200 command: '%s'%s%s", command->command,
		command->has_parameter ? " with parameter: '" : "",
		command->has_parameter ? command->parameter : "");

	return LX200_PARSE_OK;
}

// Placeholder implementations for the remaining functions
// These would need to be fully implemented based on the LX200 protocol specification

lx200_parse_result_t lx200_parse_ra_coordinate(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_ra_coordinate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement RA coordinate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_dec_coordinate(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_dec_coordinate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement Dec coordinate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_alt_coordinate(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_alt_coordinate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement Alt coordinate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_az_coordinate(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_az_coordinate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement Az coordinate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_longitude(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_longitude: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement longitude parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_latitude(const char *str, lx200_coordinate_t *coord)
{
	LOG_WRN("lx200_parse_latitude: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement latitude parsing
	ARG_UNUSED(str);
	ARG_UNUSED(coord);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_time(const char *str, lx200_time_t *time)
{
	LOG_WRN("lx200_parse_time: Function not implemented yet (str='%s')", str ? str : "NULL");
	// TODO: Implement time parsing
	ARG_UNUSED(str);
	ARG_UNUSED(time);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_date(const char *str, lx200_date_t *date)
{
	LOG_WRN("lx200_parse_date: Function not implemented yet (str='%s')", str ? str : "NULL");
	// TODO: Implement date parsing
	ARG_UNUSED(str);
	ARG_UNUSED(date);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_utc_offset(const char *str, float *offset)
{
	LOG_WRN("lx200_parse_utc_offset: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement UTC offset parsing
	ARG_UNUSED(str);
	ARG_UNUSED(offset);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_tracking_rate(const char *str, float *rate)
{
	LOG_WRN("lx200_parse_tracking_rate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement tracking rate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(rate);
	return LX200_PARSE_ERROR;
}

lx200_parse_result_t lx200_parse_slew_rate(const char *str, lx200_slew_rate_t *rate)
{
	LOG_WRN("lx200_parse_slew_rate: Function not implemented yet (str='%s')",
		str ? str : "NULL");
	// TODO: Implement slew rate parsing
	ARG_UNUSED(str);
	ARG_UNUSED(rate);
	return LX200_PARSE_ERROR;
}

int lx200_format_ra_coordinate(const lx200_coordinate_t *coord, char *str, size_t str_size)
{
	LOG_WRN("lx200_format_ra_coordinate: Function not implemented yet");
	// TODO: Implement RA coordinate formatting
	ARG_UNUSED(coord);
	ARG_UNUSED(str);
	ARG_UNUSED(str_size);
	return -1;
}

int lx200_format_dec_coordinate(const lx200_coordinate_t *coord, char *str, size_t str_size)
{
	LOG_WRN("lx200_format_dec_coordinate: Function not implemented yet");
	// TODO: Implement Dec coordinate formatting
	ARG_UNUSED(coord);
	ARG_UNUSED(str);
	ARG_UNUSED(str_size);
	return -1;
}

int lx200_format_time(const lx200_time_t *time, char *str, size_t str_size)
{
	LOG_WRN("lx200_format_time: Function not implemented yet");
	// TODO: Implement time formatting
	ARG_UNUSED(time);
	ARG_UNUSED(str);
	ARG_UNUSED(str_size);
	return -1;
}

int lx200_format_date(const lx200_date_t *date, char *str, size_t str_size)
{
	LOG_WRN("lx200_format_date: Function not implemented yet");
	// TODO: Implement date formatting
	ARG_UNUSED(date);
	ARG_UNUSED(str);
	ARG_UNUSED(str_size);
	return -1;
}

bool lx200_validate_coordinate(const lx200_coordinate_t *coord, lx200_command_family_t coord_type)
{
	LOG_WRN("lx200_validate_coordinate: Function not implemented yet (coord_type=%d)",
		coord_type);
	// TODO: Implement coordinate validation
	ARG_UNUSED(coord);
	ARG_UNUSED(coord_type);
	return false;
}

bool lx200_validate_time(const lx200_time_t *time)
{
	LOG_WRN("lx200_validate_time: Function not implemented yet");
	// TODO: Implement time validation
	ARG_UNUSED(time);
	return false;
}

bool lx200_validate_date(const lx200_date_t *date)
{
	LOG_WRN("lx200_validate_date: Function not implemented yet");
	// TODO: Implement date validation
	ARG_UNUSED(date);
	return false;
}

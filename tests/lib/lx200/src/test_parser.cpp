/*
 * LX200 Parser Tests - Result<T> API
 * Tests for the character-by-character parser with VoidResult returns
 */

#include <lx200/lx200.hpp>
#include <zephyr/ztest.h>

using namespace lx200;

ZTEST(lx200_parser, test_basic_command_parsing)
{
	ParserState parser;
	
	// Start marker
	auto result = parser.feed_character(':');
	zassert_true(result.is_ok(), "Should accept start marker");
	zassert_false(parser.is_command_ready(), "Should not be ready yet");
	
	// Command letter
	result = parser.feed_character('G');
	zassert_true(result.is_ok(), "Should accept command letter");
	zassert_false(parser.is_command_ready(), "Should not be ready yet");
	
	// Second command letter
	result = parser.feed_character('R');
	zassert_true(result.is_ok(), "Should accept second command letter");
	zassert_false(parser.is_command_ready(), "Should not be ready yet");
	
	// Terminator
	result = parser.feed_character('#');
	zassert_true(result.is_ok(), "Should accept terminator");
	zassert_true(parser.is_command_ready(), "Command should be ready");
}

ZTEST(lx200_parser, test_error_missing_start_marker)
{
	ParserState parser;
	
	// Try to feed character without start marker
	auto result = parser.feed_character('G');
	zassert_true(result.is_error(), "Should error without start marker");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_parser, test_error_empty_command)
{
	ParserState parser;
	
	// Start marker
	auto result = parser.feed_character(':');
	zassert_true(result.is_ok(), "Should accept start marker");
	
	// Immediate terminator (empty command)
	result = parser.feed_character('#');
	zassert_true(result.is_error(), "Should error on empty command");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_parser, test_buffer_full)
{
	ParserState parser;
	
	// Start marker
	auto result = parser.feed_character(':');
	zassert_true(result.is_ok(), "Should accept start marker");
	
	// Fill buffer to capacity
	for (size_t i = 1; i < ParserState::max_command_length(); i++) {
		result = parser.feed_character('A');
		zassert_true(result.is_ok(), "Should accept character %zu", i);
	}
	
	// Try to overflow
	result = parser.feed_character('X');
	zassert_true(result.is_error(), "Should error on buffer full");
	zassert_equal(result.error(), ParseError::BufferFull);
}

ZTEST(lx200_parser, test_command_extraction)
{
	ParserState parser;
	
	// Parse ":GR#"
	parser.feed_character(':');
	parser.feed_character('G');
	parser.feed_character('R');
	parser.feed_character('#');
	
	zassert_true(parser.is_command_ready(), "Command should be ready");
	
	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value(), "Should have command");
	auto command = command_opt.value();
	zassert_equal(command.name.size(), 2, "Command should be 2 characters");
	zassert_equal(command.name[0], 'G');
	zassert_equal(command.name[1], 'R');
}

ZTEST(lx200_parser, test_reset)
{
	ParserState parser;
	
	// Parse partial command
	parser.feed_character(':');
	parser.feed_character('G');
	
	// Reset
	parser.reset();
	
	// Should be able to start new command
	auto result = parser.feed_character(':');
	zassert_true(result.is_ok(), "Should accept start marker after reset");
	zassert_false(parser.is_command_ready(), "Should not be ready after reset");
}

ZTEST(lx200_parser, test_command_with_parameter)
{
	ParserState parser;
	
	// Parse ":Sr12:34:56#" (Set RA)
	parser.feed_character(':');
	parser.feed_character('S');
	parser.feed_character('r');
	parser.feed_character('1');
	parser.feed_character('2');
	parser.feed_character(':');
	parser.feed_character('3');
	parser.feed_character('4');
	parser.feed_character(':');
	parser.feed_character('5');
	parser.feed_character('6');
	parser.feed_character('#');
	
	zassert_true(parser.is_command_ready(), "Command should be ready");
	
	auto command_opt = parser.get_command();
	zassert_true(command_opt.has_value());
	auto command = command_opt.value();
	zassert_mem_equal(command.name.data(), "Sr", 2);
	zassert_mem_equal(command.parameters.data(), "12:34:56", 8);
}

ZTEST(lx200_parser, test_multiple_commands_sequentially)
{
	ParserState parser;
	
	// First command: ":GR#"
	parser.feed_character(':');
	parser.feed_character('G');
	parser.feed_character('R');
	parser.feed_character('#');
	zassert_true(parser.is_command_ready(), "First command ready");
	auto cmd1_opt = parser.get_command();
	zassert_true(cmd1_opt.has_value());
	zassert_mem_equal(cmd1_opt->name.data(), "GR", 2);
	
	// Reset for next command
	parser.reset();
	
	// Second command: ":GD#"
	parser.feed_character(':');
	parser.feed_character('G');
	parser.feed_character('D');
	parser.feed_character('#');
	zassert_true(parser.is_command_ready(), "Second command ready");
	auto cmd2_opt = parser.get_command();
	zassert_true(cmd2_opt.has_value());
	zassert_mem_equal(cmd2_opt->name.data(), "GD", 2);
}

ZTEST(lx200_parser, test_result_match_pattern_success)
{
	ParserState parser;
	auto result = parser.feed_character(':');
	
	bool success_called = false;
	bool error_called = false;
	
	result.match(
		[&](const Unit&) {
			success_called = true;
		},
		[&](ParseError) {
			error_called = true;
		}
	);
	
	zassert_true(success_called, "Success callback should be called");
	zassert_false(error_called, "Error callback should not be called");
}

ZTEST(lx200_parser, test_result_match_pattern_error)
{
	ParserState parser;
	auto result = parser.feed_character('X');  // No start marker
	
	bool success_called = false;
	bool error_called = false;
	ParseError captured_error = ParseError::General;
	
	result.match(
		[&](const Unit&) {
			success_called = true;
		},
		[&](ParseError err) {
			error_called = true;
			captured_error = err;
		}
	);
	
	zassert_false(success_called, "Success callback should not be called");
	zassert_true(error_called, "Error callback should be called");
	zassert_equal(captured_error, ParseError::InvalidFormat);
}

ZTEST_SUITE(lx200_parser, NULL, NULL, NULL, NULL, NULL);

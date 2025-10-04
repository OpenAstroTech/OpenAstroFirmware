/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <lx200/lx200.hpp>

/**
 * @file test_parser.cpp
 * @brief LX200 Parser Contract Tests
 * 
 * Tests based on parser-contract.md specifications:
 * - TC-001: Parser initialization
 * - TC-002: Parser reset
 * - TC-003: Feed valid command
 * - TC-004: Missing prefix error
 * - TC-005: Buffer overflow error
 * - TC-006: Commands with parameters
 * - TC-007: Precision mode toggle
 * - TC-008: Multiple sequential commands
 * - TC-009: Empty command error
 * - TC-010: Partial command buffering
 */

using namespace lx200;

/* ========================================================================
 * TC-001: Parser Initialization
 * ======================================================================== */

/**
 * @brief Test parser starts in known state
 * 
 * Contract: New parser has empty buffer, High precision, no command ready
 */
ZTEST(lx200, test_parser_initialization)
{
    ParserState parser;
    
    // Buffer should be empty
    zassert_false(parser.is_command_ready(), 
                  "New parser should not have command ready");
    
    // Default precision should be High
    zassert_equal(parser.get_precision(), PrecisionMode::High,
                  "Default precision should be High");
    
    // get_command() should return nullopt
    auto cmd = parser.get_command();
    zassert_false(cmd.has_value(),
                  "get_command() should return nullopt when no command ready");
}

/* ========================================================================
 * TC-002: Parser Reset
 * ======================================================================== */

/**
 * @brief Test parser reset clears state
 * 
 * Contract: reset() clears buffer, resets state, preserves precision
 */
ZTEST(lx200, test_parser_reset)
{
    ParserState parser;
    
    // Feed partial command
    zassert_ok(parser.feed_character(':'));
    zassert_ok(parser.feed_character('G'));
    zassert_ok(parser.feed_character('R'));
    
    // Set precision to Low
    parser.set_precision(PrecisionMode::Low);
    
    // Reset parser
    parser.reset();
    
    // Command should not be ready
    zassert_false(parser.is_command_ready(),
                  "Command should not be ready after reset");
    
    // Precision should be unchanged
    zassert_equal(parser.get_precision(), PrecisionMode::Low,
                  "Precision should persist after reset");
    
    // get_command() should return nullopt
    auto cmd = parser.get_command();
    zassert_false(cmd.has_value(),
                  "get_command() should return nullopt after reset");
}

/* ========================================================================
 * TC-003: Feed Valid Command
 * ======================================================================== */

/**
 * @brief Test feeding characters to build complete command
 * 
 * Contract: Command incomplete until '#', then returns Command struct
 */
ZTEST(lx200, test_feed_valid_command)
{
    ParserState parser;
    
    // Feed :GR# character by character
    zassert_ok(parser.feed_character(':'), "Should accept ':'");
    zassert_false(parser.is_command_ready(), "Not ready after ':'");
    
    zassert_ok(parser.feed_character('G'), "Should accept 'G'");
    zassert_false(parser.is_command_ready(), "Not ready after 'G'");
    
    zassert_ok(parser.feed_character('R'), "Should accept 'R'");
    zassert_false(parser.is_command_ready(), "Not ready after 'R'");
    
    zassert_ok(parser.feed_character('#'), "Should accept '#'");
    zassert_true(parser.is_command_ready(), "Should be ready after '#'");
    
    // Get command
    auto cmd = parser.get_command();
    zassert_true(cmd.has_value(), "get_command() should return Command");
    zassert_equal(cmd->family, CommandFamily::GetInfo, 
                  "Command family should be GetInfo");
    zassert_mem_equal(cmd->name.data(), "GR", 2, 
                      "Command name should be 'GR'");
}

/* ========================================================================
 * TC-004: Missing Prefix Error
 * ======================================================================== */

/**
 * @brief Test parser handles missing ':' prefix
 * 
 * Contract: First character must be ':', else error
 */
ZTEST(lx200, test_missing_prefix)
{
    ParserState parser;
    
    // Try to feed 'G' without ':' prefix
    auto result = parser.feed_character('G');
    zassert_equal(result, ParseResult::ErrorInvalidFormat,
                  "Should return error for missing prefix");
    
    // Parser should recover after reset
    parser.reset();
    zassert_ok(parser.feed_character(':'), "Should accept ':' after reset");
}

/* ========================================================================
 * TC-005: Buffer Overflow Error
 * ======================================================================== */

/**
 * @brief Test parser handles buffer overflow
 * 
 * Contract: Commands longer than MAX_COMMAND_LENGTH rejected
 */
ZTEST(lx200, test_buffer_overflow)
{
    ParserState parser;
    
    // Start command
    zassert_ok(parser.feed_character(':'));
    
    // Feed MAX_COMMAND_LENGTH characters (assuming 64 bytes)
    for (int i = 0; i < 64; i++) {
        auto result = parser.feed_character('A');
        if (result != ParseResult::Success) {
            // Should eventually get buffer overflow error
            zassert_equal(result, ParseResult::ErrorBufferFull,
                          "Should return buffer overflow error");
            return;
        }
    }
    
    ztest_test_fail("Parser should have rejected buffer overflow");
}

/* ========================================================================
 * TC-006: Commands with Parameters
 * ======================================================================== */

/**
 * @brief Test parsing commands with parameter strings
 * 
 * Contract: :Sr12:34:56# → name="Sr", params="12:34:56"
 */
ZTEST(lx200, test_command_with_parameters)
{
    ParserState parser;
    
    // Feed :Sr12:34:56#
    const char* command = ":Sr12:34:56#";
    for (const char* p = command; *p; p++) {
        zassert_ok(parser.feed_character(*p));
    }
    
    zassert_true(parser.is_command_ready(), "Command should be ready");
    
    auto cmd = parser.get_command();
    zassert_true(cmd.has_value(), "Should return Command");
    zassert_equal(cmd->family, CommandFamily::SetInfo,
                  "Family should be SetInfo");
    zassert_mem_equal(cmd->name.data(), "Sr", 2,
                      "Command name should be 'Sr'");
    zassert_mem_equal(cmd->parameters.data(), "12:34:56", 8,
                      "Parameters should be '12:34:56'");
}

/* ========================================================================
 * TC-007: Precision Mode Toggle
 * ======================================================================== */

/**
 * @brief Test precision mode switching
 * 
 * Contract: Default High, can toggle to Low, persists across commands
 */
ZTEST(lx200, test_precision_mode_toggle)
{
    ParserState parser;
    
    // Default should be High
    zassert_equal(parser.get_precision(), PrecisionMode::High,
                  "Default should be High precision");
    
    // Toggle to Low
    parser.set_precision(PrecisionMode::Low);
    zassert_equal(parser.get_precision(), PrecisionMode::Low,
                  "Should toggle to Low precision");
    
    // Process command - precision should persist
    const char* command = ":GR#";
    for (const char* p = command; *p; p++) {
        parser.feed_character(*p);
    }
    parser.get_command(); // Consume command
    
    zassert_equal(parser.get_precision(), PrecisionMode::Low,
                  "Precision should persist after command");
}

/* ========================================================================
 * TC-008: Multiple Sequential Commands
 * ======================================================================== */

/**
 * @brief Test processing multiple commands in sequence
 * 
 * Contract: Parser handles :GR#:Gd# as two separate commands
 */
ZTEST(lx200, test_multiple_sequential_commands)
{
    ParserState parser;
    
    // Feed first command :GR#
    const char* cmd1 = ":GR#";
    for (const char* p = cmd1; *p; p++) {
        parser.feed_character(*p);
    }
    
    zassert_true(parser.is_command_ready(), "First command ready");
    auto command1 = parser.get_command();
    zassert_true(command1.has_value(), "Should get first command");
    zassert_mem_equal(command1->name.data(), "GR", 2,
                      "First command should be GR");
    
    // Feed second command :Gd#
    const char* cmd2 = ":Gd#";
    for (const char* p = cmd2; *p; p++) {
        parser.feed_character(*p);
    }
    
    zassert_true(parser.is_command_ready(), "Second command ready");
    auto command2 = parser.get_command();
    zassert_true(command2.has_value(), "Should get second command");
    zassert_mem_equal(command2->name.data(), "Gd", 2,
                      "Second command should be Gd");
}

/* ========================================================================
 * TC-009: Empty Command Error
 * ======================================================================== */

/**
 * @brief Test parser rejects empty commands
 * 
 * Contract: :# should be rejected as invalid
 */
ZTEST(lx200, test_empty_command)
{
    ParserState parser;
    
    // Feed :#
    zassert_ok(parser.feed_character(':'));
    auto result = parser.feed_character('#');
    
    zassert_equal(result, ParseResult::ErrorInvalidFormat,
                  "Empty command should be rejected");
    zassert_false(parser.is_command_ready(),
                  "No command should be ready after error");
}

/* ========================================================================
 * TC-010: Partial Command Buffering
 * ======================================================================== */

/**
 * @brief Test parser buffers partial commands correctly
 * 
 * Contract: Command stays incomplete until terminator '#'
 */
ZTEST(lx200, test_partial_command_buffering)
{
    ParserState parser;
    
    // Feed partial command :GR (no terminator)
    zassert_ok(parser.feed_character(':'));
    zassert_ok(parser.feed_character('G'));
    zassert_ok(parser.feed_character('R'));
    
    // Command should not be ready
    zassert_false(parser.is_command_ready(),
                  "Partial command should not be ready");
    
    auto cmd = parser.get_command();
    zassert_false(cmd.has_value(),
                  "get_command() should return nullopt for partial command");
    
    // Complete the command
    zassert_ok(parser.feed_character('#'));
    zassert_true(parser.is_command_ready(),
                 "Command should be ready after terminator");
}

/* ========================================================================
 * Test Suite Registration
 * ======================================================================== */

extern "C" void test_suite_parser(void)
{
    ztest_run_test_suites(NULL);
}

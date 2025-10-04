/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

/**
 * @brief LX200 C++20 Test Suite
 *
 * This test suite validates the C++20 rewrite of the LX200 telescope
 * control protocol parser. Tests are organized by component:
 *
 * - Parser tests: Command parsing, state management
 * - Coordinate tests: RA/DEC/ALT/AZ/LAT/LON parsing
 * - Command tests: Command family identification
 * - Integration tests: End-to-end protocol validation
 */

/* Test suite declarations - defined in separate files */
extern "C" {
void test_suite_parser(void);
void test_suite_coordinates(void);
void test_suite_commands(void);
void test_suite_integration(void);
}

/**
 * @brief Main test entry point
 *
 * Zephyr test runner will execute all registered test suites.
 * Each suite is defined in its own source file for modularity.
 */
ZTEST_SUITE(lx200, NULL, NULL, NULL, NULL, NULL);

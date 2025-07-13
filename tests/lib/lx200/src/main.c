/*
 * Copyright (c) 2021 Legrand North America, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * @file test custom_lib library
 *
 * This suite verifies that the methods provided with the custom_lib
 * library works correctly.
 */

#include <limits.h>

#include <zephyr/ztest.h>

#include <lx200/lx200.h>

ZTEST(lx200, test_dummy)
{
	zassert_equal(0, 0, "Dummy test failed");
}

ZTEST_SUITE(lx200, NULL, NULL, NULL, NULL, NULL);

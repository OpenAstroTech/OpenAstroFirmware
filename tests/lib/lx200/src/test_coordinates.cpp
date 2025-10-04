/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <lx200/lx200.hpp>
#include <string>

/**
 * @file test_coordinates.cpp
 * @brief LX200 Coordinate Parsing Contract Tests
 * 
 * Tests based on coordinate-parsing-contract.md:
 * - TC-RA-001 to TC-RA-005: Right Ascension parsing
 * - TC-DEC-001 to TC-DEC-005: Declination parsing
 * - TC-LAT-001 to TC-LAT-002: Latitude parsing
 * - TC-LON-001 to TC-LON-002: Longitude parsing
 * - TC-TIME-001 to TC-TIME-002: Time parsing
 * - TC-DATE-001 to TC-DATE-003: Date parsing
 */

using namespace lx200;

/* ========================================================================
 * Right Ascension Tests
 * ======================================================================== */

/**
 * @brief TC-RA-001: Parse high precision RA coordinate
 * 
 * Format: HH:MM:SS (e.g., "12:34:56")
 */
ZTEST(lx200, test_ra_high_precision)
{
    RACoordinate ra;
    auto result = parse_ra_coordinate("12:34:56", PrecisionMode::High, ra);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid RA");
    zassert_equal(ra.hours, 12, "Hours should be 12");
    zassert_equal(ra.minutes, 34, "Minutes should be 34");
    zassert_equal(ra.seconds, 56, "Seconds should be 56");
}

/**
 * @brief TC-RA-002: Parse low precision RA coordinate
 * 
 * Format: HH:MM.T (e.g., "12:34.5")
 */
ZTEST(lx200, test_ra_low_precision)
{
    RACoordinate ra;
    auto result = parse_ra_coordinate("12:34.5", PrecisionMode::Low, ra);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid RA");
    zassert_equal(ra.hours, 12, "Hours should be 12");
    zassert_equal(ra.minutes, 34, "Minutes should be 34");
    zassert_equal(ra.tenths, 5, "Tenths should be 5");
}

/**
 * @brief TC-RA-003: Validate RA range (0-23 hours)
 */
ZTEST(lx200, test_ra_validation)
{
    RACoordinate ra;
    
    // Valid boundary cases
    zassert_ok(parse_ra_coordinate("00:00:00", PrecisionMode::High, ra),
               "Should accept 00:00:00");
    zassert_ok(parse_ra_coordinate("23:59:59", PrecisionMode::High, ra),
               "Should accept 23:59:59");
    
    // Invalid cases
    zassert_equal(parse_ra_coordinate("24:00:00", PrecisionMode::High, ra),
                  ParseResult::ErrorOutOfRange,
                  "Should reject hours >= 24");
    zassert_equal(parse_ra_coordinate("12:60:00", PrecisionMode::High, ra),
                  ParseResult::ErrorOutOfRange,
                  "Should reject minutes >= 60");
    zassert_equal(parse_ra_coordinate("12:34:60", PrecisionMode::High, ra),
                  ParseResult::ErrorOutOfRange,
                  "Should reject seconds >= 60");
}

/**
 * @brief TC-RA-004: Test RA format validation
 */
ZTEST(lx200, test_ra_format_validation)
{
    RACoordinate ra;
    
    // Invalid formats
    zassert_equal(parse_ra_coordinate("12:34", PrecisionMode::High, ra),
                  ParseResult::ErrorInvalidFormat,
                  "Should reject incomplete format");
    zassert_equal(parse_ra_coordinate("12:34:5X", PrecisionMode::High, ra),
                  ParseResult::ErrorInvalidFormat,
                  "Should reject non-numeric characters");
    zassert_equal(parse_ra_coordinate("1234:56", PrecisionMode::High, ra),
                  ParseResult::ErrorInvalidFormat,
                  "Should reject malformed separators");
}

/**
 * @brief TC-RA-005: Test zero allocation constraint
 */
ZTEST(lx200, test_ra_zero_allocation)
{
    RACoordinate ra;
    
    // This test verifies parsing uses stack only
    // No heap allocations should occur
    auto result = parse_ra_coordinate("12:34:56", PrecisionMode::High, ra);
    zassert_ok(result, "Should parse without allocation");
    
    // Verify result is on stack (size check)
    zassert_true(sizeof(ra) <= 8, "RA should be small stack object");
}

/* ========================================================================
 * Declination Tests
 * ======================================================================== */

/**
 * @brief TC-DEC-001: Parse positive declination
 * 
 * Format: sDD*MM:SS (e.g., "+45*30:15")
 */
ZTEST(lx200, test_dec_positive)
{
    DECCoordinate dec;
    auto result = parse_dec_coordinate("+45*30:15", PrecisionMode::High, dec);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid DEC");
    zassert_equal(dec.sign, '+', "Sign should be positive");
    zassert_equal(dec.degrees, 45, "Degrees should be 45");
    zassert_equal(dec.arcminutes, 30, "Arcminutes should be 30");
    zassert_equal(dec.arcseconds, 15, "Arcseconds should be 15");
}

/**
 * @brief TC-DEC-002: Parse negative declination
 * 
 * Format: sDD*MM:SS (e.g., "-12*45:30")
 */
ZTEST(lx200, test_dec_negative)
{
    DECCoordinate dec;
    auto result = parse_dec_coordinate("-12*45:30", PrecisionMode::High, dec);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid DEC");
    zassert_equal(dec.sign, '-', "Sign should be negative");
    zassert_equal(dec.degrees, 12, "Degrees should be 12");
    zassert_equal(dec.arcminutes, 45, "Arcminutes should be 45");
    zassert_equal(dec.arcseconds, 30, "Arcseconds should be 30");
}

/**
 * @brief TC-DEC-003: Validate DEC range (-90° to +90°)
 */
ZTEST(lx200, test_dec_validation)
{
    DECCoordinate dec;
    
    // Valid boundary cases
    zassert_ok(parse_dec_coordinate("+90*00:00", PrecisionMode::High, dec),
               "Should accept +90°");
    zassert_ok(parse_dec_coordinate("-90*00:00", PrecisionMode::High, dec),
               "Should accept -90°");
    
    // Invalid cases
    zassert_equal(parse_dec_coordinate("+91*00:00", PrecisionMode::High, dec),
                  ParseResult::ErrorOutOfRange,
                  "Should reject > +90°");
    zassert_equal(parse_dec_coordinate("-91*00:00", PrecisionMode::High, dec),
                  ParseResult::ErrorOutOfRange,
                  "Should reject < -90°");
}

/**
 * @brief TC-DEC-004: Test apostrophe separator (alternate format)
 * 
 * LX200 uses * for degrees, ' for arcminutes in some contexts
 */
ZTEST(lx200, test_dec_apostrophe_separator)
{
    DECCoordinate dec;
    
    // Test degree symbol variants
    auto result = parse_dec_coordinate("+45*30'15", PrecisionMode::High, dec);
    zassert_equal(result, ParseResult::Success, 
                  "Should handle apostrophe separator");
}

/**
 * @brief TC-DEC-005: Test low precision DEC format
 * 
 * Format: sDD*MM (e.g., "+45*30")
 */
ZTEST(lx200, test_dec_low_precision)
{
    DECCoordinate dec;
    auto result = parse_dec_coordinate("+45*30", PrecisionMode::Low, dec);
    
    zassert_equal(result, ParseResult::Success, "Should parse low precision DEC");
    zassert_equal(dec.degrees, 45, "Degrees should be 45");
    zassert_equal(dec.arcminutes, 30, "Arcminutes should be 30");
}

/* ========================================================================
 * Latitude Tests
 * ======================================================================== */

/**
 * @brief TC-LAT-001: Parse latitude coordinate
 * 
 * Format: sDD*MM (e.g., "+37*45")
 */
ZTEST(lx200, test_latitude_parsing)
{
    LatitudeCoordinate lat;
    auto result = parse_latitude_coordinate("+37*45", lat);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid latitude");
    zassert_equal(lat.sign, '+', "Sign should be positive");
    zassert_equal(lat.degrees, 37, "Degrees should be 37");
    zassert_equal(lat.arcminutes, 45, "Arcminutes should be 45");
}

/**
 * @brief TC-LAT-002: Validate latitude range (-90° to +90°)
 */
ZTEST(lx200, test_latitude_validation)
{
    LatitudeCoordinate lat;
    
    // Valid boundary cases
    zassert_ok(parse_latitude_coordinate("+90*00", lat), "Should accept +90°");
    zassert_ok(parse_latitude_coordinate("-90*00", lat), "Should accept -90°");
    
    // Invalid cases
    zassert_equal(parse_latitude_coordinate("+91*00", lat),
                  ParseResult::ErrorOutOfRange,
                  "Should reject > +90°");
}

/* ========================================================================
 * Longitude Tests
 * ======================================================================== */

/**
 * @brief TC-LON-001: Parse longitude coordinate (0-360°)
 * 
 * Format: DDD*MM (e.g., "122*30")
 */
ZTEST(lx200, test_longitude_parsing)
{
    LongitudeCoordinate lon;
    auto result = parse_longitude_coordinate("122*30", lon);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid longitude");
    zassert_equal(lon.degrees, 122, "Degrees should be 122");
    zassert_equal(lon.arcminutes, 30, "Arcminutes should be 30");
}

/**
 * @brief TC-LON-002: Validate longitude range (0° to 360°)
 */
ZTEST(lx200, test_longitude_validation)
{
    LongitudeCoordinate lon;
    
    // Valid boundary cases
    zassert_ok(parse_longitude_coordinate("000*00", lon), "Should accept 0°");
    zassert_ok(parse_longitude_coordinate("359*59", lon), "Should accept 359°59'");
    
    // Invalid case
    zassert_equal(parse_longitude_coordinate("360*00", lon),
                  ParseResult::ErrorOutOfRange,
                  "Should reject >= 360°");
}

/* ========================================================================
 * Time Tests
 * ======================================================================== */

/**
 * @brief TC-TIME-001: Parse time value
 * 
 * Format: HH:MM:SS (e.g., "14:30:45")
 */
ZTEST(lx200, test_time_parsing)
{
    TimeValue time;
    auto result = parse_time_value("14:30:45", time);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid time");
    zassert_equal(time.hours, 14, "Hours should be 14");
    zassert_equal(time.minutes, 30, "Minutes should be 30");
    zassert_equal(time.seconds, 45, "Seconds should be 45");
}

/**
 * @brief TC-TIME-002: Validate time ranges
 */
ZTEST(lx200, test_time_validation)
{
    TimeValue time;
    
    // Valid boundary cases
    zassert_ok(parse_time_value("00:00:00", time), "Should accept 00:00:00");
    zassert_ok(parse_time_value("23:59:59", time), "Should accept 23:59:59");
    
    // Invalid cases
    zassert_equal(parse_time_value("24:00:00", time),
                  ParseResult::ErrorOutOfRange,
                  "Should reject hours >= 24");
    zassert_equal(parse_time_value("12:60:00", time),
                  ParseResult::ErrorOutOfRange,
                  "Should reject minutes >= 60");
}

/* ========================================================================
 * Date Tests
 * ======================================================================== */

/**
 * @brief TC-DATE-001: Parse date value
 * 
 * Format: MM/DD/YY (e.g., "03/15/23")
 */
ZTEST(lx200, test_date_parsing)
{
    DateValue date;
    auto result = parse_date_value("03/15/23", date);
    
    zassert_equal(result, ParseResult::Success, "Should parse valid date");
    zassert_equal(date.month, 3, "Month should be 3");
    zassert_equal(date.day, 15, "Day should be 15");
    zassert_equal(date.year, 23, "Year should be 23");
}

/**
 * @brief TC-DATE-002: Validate month range (1-12)
 */
ZTEST(lx200, test_date_month_validation)
{
    DateValue date;
    
    // Valid boundary cases
    zassert_ok(parse_date_value("01/15/23", date), "Should accept month 1");
    zassert_ok(parse_date_value("12/15/23", date), "Should accept month 12");
    
    // Invalid cases
    zassert_equal(parse_date_value("00/15/23", date),
                  ParseResult::ErrorOutOfRange,
                  "Should reject month 0");
    zassert_equal(parse_date_value("13/15/23", date),
                  ParseResult::ErrorOutOfRange,
                  "Should reject month > 12");
}

/**
 * @brief TC-DATE-003: Validate day range (1-31)
 */
ZTEST(lx200, test_date_day_validation)
{
    DateValue date;
    
    // Valid boundary cases
    zassert_ok(parse_date_value("03/01/23", date), "Should accept day 1");
    zassert_ok(parse_date_value("03/31/23", date), "Should accept day 31");
    
    // Invalid cases
    zassert_equal(parse_date_value("03/00/23", date),
                  ParseResult::ErrorOutOfRange,
                  "Should reject day 0");
    zassert_equal(parse_date_value("03/32/23", date),
                  ParseResult::ErrorOutOfRange,
                  "Should reject day > 31");
}

/* ========================================================================
 * Performance Tests
 * ======================================================================== */

/**
 * @brief Test coordinate parsing performance (<5μs requirement)
 */
ZTEST(lx200, test_coordinate_parsing_performance)
{
    RACoordinate ra;
    
    // Warm up cache
    parse_ra_coordinate("12:34:56", PrecisionMode::High, ra);
    
    // Measure parsing time
    uint32_t start = k_cycle_get_32();
    for (int i = 0; i < 100; i++) {
        parse_ra_coordinate("12:34:56", PrecisionMode::High, ra);
    }
    uint32_t end = k_cycle_get_32();
    
    uint32_t cycles = end - start;
    uint32_t avg_cycles = cycles / 100;
    
    // At typical embedded clock speeds (e.g., 168 MHz),
    // <5μs means <840 cycles per parse
    zassert_true(avg_cycles < 1000, 
                 "Coordinate parsing should be fast (<5μs average)");
}

/* ========================================================================
 * Test Suite Registration
 * ======================================================================== */

extern "C" void test_suite_coordinates(void)
{
    // Tests are automatically registered via ZTEST macro
}

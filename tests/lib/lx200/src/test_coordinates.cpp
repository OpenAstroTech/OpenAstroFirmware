/*
 * LX200 Coordinate Parsing Tests - Result<T> API
 * Tests for all coordinate parsing functions returning Result<T>
 */

#include <lx200/lx200.hpp>
#include <zephyr/ztest.h>

using namespace lx200;

/* ========================================================================
 * Right Ascension (RA) Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_ra_high_precision_valid)
{
	auto result = parse_ra("12:34:56", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse valid RA");
	
	auto coord = result.value();
	zassert_equal(coord.hours, 12, "Hours should be 12");
	zassert_equal(coord.minutes, 34, "Minutes should be 34");
	zassert_equal(coord.seconds, 56, "Seconds should be 56");
}

ZTEST(lx200_coordinates, test_ra_low_precision_valid)
{
	auto result = parse_ra("23:59.9", PrecisionMode::Low);
	
	zassert_true(result.is_ok(), "Should parse valid RA in low precision");
	
	auto coord = result.value();
	zassert_equal(coord.hours, 23, "Hours should be 23");
	zassert_equal(coord.minutes, 59, "Minutes should be 59");
	zassert_equal(coord.seconds, 54, "Seconds should be 54 (9*6)");
}

ZTEST(lx200_coordinates, test_ra_zero_values)
{
	auto result = parse_ra("00:00:00", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse zero RA");
	
	auto coord = result.value();
	zassert_equal(coord.hours, 0);
	zassert_equal(coord.minutes, 0);
	zassert_equal(coord.seconds, 0);
}

ZTEST(lx200_coordinates, test_ra_max_valid_values)
{
	auto result = parse_ra("23:59:59", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse max valid RA");
	
	auto coord = result.value();
	zassert_equal(coord.hours, 23);
	zassert_equal(coord.minutes, 59);
	zassert_equal(coord.seconds, 59);
}

ZTEST(lx200_coordinates, test_ra_hours_out_of_range)
{
	auto result = parse_ra("24:00:00", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on hours >= 24");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_ra_minutes_out_of_range)
{
	auto result = parse_ra("12:60:00", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on minutes >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_ra_seconds_out_of_range)
{
	auto result = parse_ra("12:34:60", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on seconds >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_ra_empty_string)
{
	auto result = parse_ra("", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on empty string");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_ra_invalid_format)
{
	auto result = parse_ra("invalid", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on invalid format");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_ra_match_pattern)
{
	auto result = parse_ra("10:20:30", PrecisionMode::High);
	
	bool success = false;
	result.match(
		[&](const RACoordinate& coord) {
			success = true;
			zassert_equal(coord.hours, 10);
			zassert_equal(coord.minutes, 20);
			zassert_equal(coord.seconds, 30);
		},
		[](ParseError) {
			zassert_unreachable("Should not call error callback");
		}
	);
	
	zassert_true(success, "Match should call success callback");
}

ZTEST(lx200_coordinates, test_ra_value_or)
{
	auto error_result = parse_ra("invalid", PrecisionMode::High);
	
	RACoordinate default_coord{1, 2, 3};
	auto coord = error_result.value_or(default_coord);
	
	zassert_equal(coord.hours, 1, "Should return default hours");
	zassert_equal(coord.minutes, 2, "Should return default minutes");
	zassert_equal(coord.seconds, 3, "Should return default seconds");
}

/* ========================================================================
 * Declination (DEC) Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_dec_positive_high_precision)
{
	auto result = parse_dec("+45*30:15", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse positive DEC");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 45);
	zassert_equal(coord.arcminutes, 30);
	zassert_equal(coord.arcseconds, 15);
}

ZTEST(lx200_coordinates, test_dec_negative_high_precision)
{
	auto result = parse_dec("-89*59:59", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse negative DEC");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '-');
	zassert_equal(coord.degrees, 89);
	zassert_equal(coord.arcminutes, 59);
	zassert_equal(coord.arcseconds, 59);
}

ZTEST(lx200_coordinates, test_dec_low_precision)
{
	// Low precision DEC format: sDD*MM (no tenths support yet)
	auto result = parse_dec("+45*30", PrecisionMode::Low);
	
	zassert_true(result.is_ok(), "Should parse DEC in low precision");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 45);
	zassert_equal(coord.arcminutes, 30);
	zassert_equal(coord.arcseconds, 0, "Low precision has no arcseconds");
}

ZTEST(lx200_coordinates, test_dec_zero)
{
	auto result = parse_dec("+00*00:00", PrecisionMode::High);
	
	zassert_true(result.is_ok(), "Should parse zero DEC");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 0);
	zassert_equal(coord.arcminutes, 0);
	zassert_equal(coord.arcseconds, 0);
}

ZTEST(lx200_coordinates, test_dec_degrees_out_of_range)
{
	auto result = parse_dec("+90*00:01", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on degrees > 90");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_dec_minutes_out_of_range)
{
	auto result = parse_dec("+45*60:00", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on minutes >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_dec_invalid_sign)
{
	auto result = parse_dec("X45*30:15", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on invalid sign");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_dec_empty_string)
{
	auto result = parse_dec("", PrecisionMode::High);
	
	zassert_true(result.is_error(), "Should error on empty string");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

/* ========================================================================
 * Latitude Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_latitude_positive)
{
	auto result = parse_latitude("+45*30");
	
	zassert_true(result.is_ok(), "Should parse positive latitude");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 45);
	zassert_equal(coord.arcminutes, 30);
}

ZTEST(lx200_coordinates, test_latitude_negative)
{
	auto result = parse_latitude("-33*52");
	
	zassert_true(result.is_ok(), "Should parse negative latitude");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '-');
	zassert_equal(coord.degrees, 33);
	zassert_equal(coord.arcminutes, 52);
}

ZTEST(lx200_coordinates, test_latitude_zero)
{
	auto result = parse_latitude("+00*00");
	
	zassert_true(result.is_ok(), "Should parse zero latitude");
	
	auto coord = result.value();
	zassert_equal(coord.sign, '+');
	zassert_equal(coord.degrees, 0);
	zassert_equal(coord.arcminutes, 0);
}

ZTEST(lx200_coordinates, test_latitude_max_valid)
{
	auto result = parse_latitude("+90*00");
	
	zassert_true(result.is_ok(), "Should parse +90 degrees");
	
	auto coord = result.value();
	zassert_equal(coord.degrees, 90);
}

ZTEST(lx200_coordinates, test_latitude_out_of_range)
{
	auto result = parse_latitude("+91*00");
	
	zassert_true(result.is_error(), "Should error on latitude > 90");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_latitude_minutes_out_of_range)
{
	auto result = parse_latitude("+45*60");
	
	zassert_true(result.is_error(), "Should error on minutes >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_latitude_invalid_format)
{
	auto result = parse_latitude("invalid");
	
	zassert_true(result.is_error(), "Should error on invalid format");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

/* ========================================================================
 * Longitude Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_longitude_positive)
{
	auto result = parse_longitude("151*12");
	
	zassert_true(result.is_ok(), "Should parse longitude");
	
	auto coord = result.value();
	zassert_equal(coord.degrees, 151);
	zassert_equal(coord.arcminutes, 12);
}

ZTEST(lx200_coordinates, test_longitude_negative)
{
	auto result = parse_longitude("122*25");
	
	zassert_true(result.is_ok(), "Should parse longitude");
	
	auto coord = result.value();
	zassert_equal(coord.degrees, 122);
	zassert_equal(coord.arcminutes, 25);
}

ZTEST(lx200_coordinates, test_longitude_zero)
{
	auto result = parse_longitude("000*00");
	
	zassert_true(result.is_ok(), "Should parse zero longitude");
	
	auto coord = result.value();
	zassert_equal(coord.degrees, 0);
	zassert_equal(coord.arcminutes, 0);
}

ZTEST(lx200_coordinates, test_longitude_max_valid)
{
	auto result = parse_longitude("180*00");
	
	zassert_true(result.is_ok(), "Should parse 180 degrees");
	
	auto coord = result.value();
	zassert_equal(coord.degrees, 180);
}

ZTEST(lx200_coordinates, test_longitude_out_of_range)
{
	auto result = parse_longitude("360*00");
	
	zassert_true(result.is_error(), "Should error on longitude >= 360");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_longitude_minutes_out_of_range)
{
	auto result = parse_longitude("100*60");
	
	zassert_true(result.is_error(), "Should error on arcminutes >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_longitude_invalid_format)
{
	auto result = parse_longitude("not-valid");
	
	zassert_true(result.is_error(), "Should error on invalid format");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

/* ========================================================================
 * Time Value Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_time_valid)
{
	auto result = parse_time("14:30:45");
	
	zassert_true(result.is_ok(), "Should parse valid time");
	
	auto time = result.value();
	zassert_equal(time.hours, 14);
	zassert_equal(time.minutes, 30);
	zassert_equal(time.seconds, 45);
}

ZTEST(lx200_coordinates, test_time_midnight)
{
	auto result = parse_time("00:00:00");
	
	zassert_true(result.is_ok(), "Should parse midnight");
	
	auto time = result.value();
	zassert_equal(time.hours, 0);
	zassert_equal(time.minutes, 0);
	zassert_equal(time.seconds, 0);
}

ZTEST(lx200_coordinates, test_time_max_valid)
{
	auto result = parse_time("23:59:59");
	
	zassert_true(result.is_ok(), "Should parse max valid time");
	
	auto time = result.value();
	zassert_equal(time.hours, 23);
	zassert_equal(time.minutes, 59);
	zassert_equal(time.seconds, 59);
}

ZTEST(lx200_coordinates, test_time_hours_out_of_range)
{
	auto result = parse_time("24:00:00");
	
	zassert_true(result.is_error(), "Should error on hours >= 24");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_time_minutes_out_of_range)
{
	auto result = parse_time("12:60:00");
	
	zassert_true(result.is_error(), "Should error on minutes >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_time_seconds_out_of_range)
{
	auto result = parse_time("12:30:60");
	
	zassert_true(result.is_error(), "Should error on seconds >= 60");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_time_invalid_format)
{
	auto result = parse_time("not-a-time");
	
	zassert_true(result.is_error(), "Should error on invalid format");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_time_empty_string)
{
	auto result = parse_time("");
	
	zassert_true(result.is_error(), "Should error on empty string");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

/* ========================================================================
 * Date Value Tests
 * ======================================================================== */

ZTEST(lx200_coordinates, test_date_valid)
{
	auto result = parse_date("12/25/23");
	
	zassert_true(result.is_ok(), "Should parse valid date");
	
	auto date = result.value();
	zassert_equal(date.month, 12);
	zassert_equal(date.day, 25);
	zassert_equal(date.year, 23);
}

ZTEST(lx200_coordinates, test_date_january_first)
{
	auto result = parse_date("01/01/00");
	
	zassert_true(result.is_ok(), "Should parse Jan 1");
	
	auto date = result.value();
	zassert_equal(date.month, 1);
	zassert_equal(date.day, 1);
	zassert_equal(date.year, 0);
}

ZTEST(lx200_coordinates, test_date_december_thirty_first)
{
	auto result = parse_date("12/31/99");
	
	zassert_true(result.is_ok(), "Should parse Dec 31");
	
	auto date = result.value();
	zassert_equal(date.month, 12);
	zassert_equal(date.day, 31);
	zassert_equal(date.year, 99);
}

ZTEST(lx200_coordinates, test_date_month_out_of_range_zero)
{
	auto result = parse_date("00/15/23");
	
	zassert_true(result.is_error(), "Should error on month 0");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_month_out_of_range_high)
{
	auto result = parse_date("13/15/23");
	
	zassert_true(result.is_error(), "Should error on month > 12");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_day_out_of_range_zero)
{
	auto result = parse_date("06/00/23");
	
	zassert_true(result.is_error(), "Should error on day 0");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_day_out_of_range_high)
{
	auto result = parse_date("06/32/23");
	
	zassert_true(result.is_error(), "Should error on day > 31");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_year_out_of_range)
{
	// Year must be exactly 2 digits, so 3-digit year is invalid format
	auto result = parse_date("06/15/100");
	
	zassert_true(result.is_error(), "Should error on 3-digit year");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_date_invalid_format)
{
	auto result = parse_date("invalid-date");
	
	zassert_true(result.is_error(), "Should error on invalid format");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

ZTEST(lx200_coordinates, test_date_empty_string)
{
	auto result = parse_date("");
	
	zassert_true(result.is_error(), "Should error on empty string");
	zassert_equal(result.error(), ParseError::InvalidFormat);
}

/* ========================================================================
 * Date Validation - Month-Specific Day Limits
 * ======================================================================== */

ZTEST(lx200_coordinates, test_date_february_30_invalid)
{
	// February has only 28/29 days
	auto result = parse_date("02/30/25");
	
	zassert_true(result.is_error(), "Feb 30 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_february_29_non_leap_year)
{
	// 2023 is not a leap year
	auto result = parse_date("02/29/23");
	
	zassert_true(result.is_error(), "Feb 29 in non-leap year should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_february_29_leap_year_valid)
{
	// 2024 is a leap year
	auto result = parse_date("02/29/24");
	
	zassert_true(result.is_ok(), "Feb 29 in leap year should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 29);
	zassert_equal(date.year, 24);
}

ZTEST(lx200_coordinates, test_date_february_28_non_leap_year_valid)
{
	// Feb 28 should always be valid
	auto result = parse_date("02/28/23");
	
	zassert_true(result.is_ok(), "Feb 28 should always be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 28);
	zassert_equal(date.year, 23);
}

ZTEST(lx200_coordinates, test_date_april_31_invalid)
{
	// April has only 30 days
	auto result = parse_date("04/31/25");
	
	zassert_true(result.is_error(), "April 31 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_april_30_valid)
{
	// April 30 is valid
	auto result = parse_date("04/30/25");
	
	zassert_true(result.is_ok(), "April 30 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 4);
	zassert_equal(date.day, 30);
}

ZTEST(lx200_coordinates, test_date_june_31_invalid)
{
	// June has only 30 days
	auto result = parse_date("06/31/25");
	
	zassert_true(result.is_error(), "June 31 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_june_30_valid)
{
	// June 30 is valid
	auto result = parse_date("06/30/25");
	
	zassert_true(result.is_ok(), "June 30 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 6);
	zassert_equal(date.day, 30);
}

ZTEST(lx200_coordinates, test_date_september_31_invalid)
{
	// September has only 30 days
	auto result = parse_date("09/31/25");
	
	zassert_true(result.is_error(), "September 31 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_september_30_valid)
{
	// September 30 is valid
	auto result = parse_date("09/30/25");
	
	zassert_true(result.is_ok(), "September 30 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 9);
	zassert_equal(date.day, 30);
}

ZTEST(lx200_coordinates, test_date_november_31_invalid)
{
	// November has only 30 days
	auto result = parse_date("11/31/25");
	
	zassert_true(result.is_error(), "November 31 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_november_30_valid)
{
	// November 30 is valid
	auto result = parse_date("11/30/25");
	
	zassert_true(result.is_ok(), "November 30 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 11);
	zassert_equal(date.day, 30);
}

ZTEST(lx200_coordinates, test_date_january_31_valid)
{
	// January has 31 days
	auto result = parse_date("01/31/25");
	
	zassert_true(result.is_ok(), "January 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 1);
	zassert_equal(date.day, 31);
}

ZTEST(lx200_coordinates, test_date_march_31_valid)
{
	// March has 31 days
	auto result = parse_date("03/31/25");
	
	zassert_true(result.is_ok(), "March 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 3);
	zassert_equal(date.day, 31);
}

ZTEST(lx200_coordinates, test_date_may_31_valid)
{
	// May has 31 days
	auto result = parse_date("05/31/25");
	
	zassert_true(result.is_ok(), "May 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 5);
	zassert_equal(date.day, 31);
}

ZTEST(lx200_coordinates, test_date_july_31_valid)
{
	// July has 31 days
	auto result = parse_date("07/31/25");
	
	zassert_true(result.is_ok(), "July 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 7);
	zassert_equal(date.day, 31);
}

ZTEST(lx200_coordinates, test_date_august_31_valid)
{
	// August has 31 days
	auto result = parse_date("08/31/25");
	
	zassert_true(result.is_ok(), "August 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 8);
	zassert_equal(date.day, 31);
}

ZTEST(lx200_coordinates, test_date_october_31_valid)
{
	// October has 31 days
	auto result = parse_date("10/31/25");
	
	zassert_true(result.is_ok(), "October 31 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 10);
	zassert_equal(date.day, 31);
}

/* ========================================================================
 * Date Validation - Leap Year Boundary Cases
 * ======================================================================== */

ZTEST(lx200_coordinates, test_date_leap_year_2020)
{
	// 2020 is a leap year (divisible by 4)
	auto result = parse_date("02/29/20");
	
	zassert_true(result.is_ok(), "2020 is a leap year, Feb 29 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 29);
	zassert_equal(date.year, 20);
}

ZTEST(lx200_coordinates, test_date_leap_year_2000)
{
	// 2000 is a leap year (divisible by 400)
	auto result = parse_date("02/29/00");
	
	zassert_true(result.is_ok(), "2000 is a leap year, Feb 29 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 29);
	zassert_equal(date.year, 0);
}

ZTEST(lx200_coordinates, test_date_non_leap_year_2021)
{
	// 2021 is not a leap year
	auto result = parse_date("02/29/21");
	
	zassert_true(result.is_error(), "2021 is not a leap year, Feb 29 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_non_leap_year_2022)
{
	// 2022 is not a leap year
	auto result = parse_date("02/29/22");
	
	zassert_true(result.is_error(), "2022 is not a leap year, Feb 29 should be invalid");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST(lx200_coordinates, test_date_leap_year_2028)
{
	// 2028 is a leap year (divisible by 4)
	auto result = parse_date("02/29/28");
	
	zassert_true(result.is_ok(), "2028 is a leap year, Feb 29 should be valid");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 29);
	zassert_equal(date.year, 28);
}

ZTEST(lx200_coordinates, test_date_february_28_leap_year_valid)
{
	// Feb 28 should be valid even in leap years
	auto result = parse_date("02/28/24");
	
	zassert_true(result.is_ok(), "Feb 28 should be valid in leap years");
	
	auto date = result.value();
	zassert_equal(date.month, 2);
	zassert_equal(date.day, 28);
	zassert_equal(date.year, 24);
}

ZTEST(lx200_coordinates, test_date_february_30_leap_year_invalid)
{
	// Feb 30 is never valid, even in leap years
	auto result = parse_date("02/30/24");
	
	zassert_true(result.is_error(), "Feb 30 should be invalid even in leap years");
	zassert_equal(result.error(), ParseError::OutOfRange);
}

ZTEST_SUITE(lx200_coordinates, NULL, NULL, NULL, NULL, NULL);

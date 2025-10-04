/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lx200/lx200.hpp>
#include <cstdlib>
#include <cstring>

namespace lx200 {

/* ========================================================================
 * Helper Functions
 * ======================================================================== */

/**
 * @brief Parse unsigned integer from string
 * 
 * @param str Input string
 * @param len Maximum characters to parse
 * @param[out] value Parsed value
 * @return true if parsing succeeded
 */
static bool parse_uint(const char* str, size_t len, uint32_t& value) noexcept
{
    if (!str || len == 0) return false;
    
    value = 0;
    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
        value = value * 10 + (str[i] - '0');
    }
    return true;
}

/**
 * @brief Find character in string
 */
static const char* find_char(const char* str, char c) noexcept
{
    if (!str) return nullptr;
    while (*str && *str != c) str++;
    return (*str == c) ? str : nullptr;
}

/* ========================================================================
 * Right Ascension Parsing
 * ======================================================================== */

ParseResult parse_ra_coordinate(
    const char* str,
    PrecisionMode mode,
    RACoordinate& coord
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: HH:MM:SS (high) or HH:MM.T (low)
    const char* colon1 = find_char(str, ':');
    if (!colon1) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse hours
    uint32_t hours;
    if (!parse_uint(str, colon1 - str, hours) || hours >= 24) {
        return ParseResult::ErrorOutOfRange;
    }
    
    const char* minute_start = colon1 + 1;
    
    if (mode == PrecisionMode::High) {
        // High precision: HH:MM:SS
        const char* colon2 = find_char(minute_start, ':');
        if (!colon2) {
            return ParseResult::ErrorInvalidFormat;
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_start, colon2 - minute_start, minutes) || minutes >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        // Parse seconds
        uint32_t seconds;
        const char* second_start = colon2 + 1;
        if (!parse_uint(second_start, 2, seconds) || seconds >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        coord.hours = static_cast<uint8_t>(hours);
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = static_cast<uint8_t>(seconds);
        coord.tenths = 0;
    }
    else {
        // Low precision: HH:MM.T
        const char* dot = find_char(minute_start, '.');
        if (!dot) {
            return ParseResult::ErrorInvalidFormat;
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_start, dot - minute_start, minutes) || minutes >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        // Parse tenths
        uint32_t tenths;
        const char* tenth_start = dot + 1;
        if (!parse_uint(tenth_start, 1, tenths) || tenths >= 10) {
            return ParseResult::ErrorOutOfRange;
        }
        
        coord.hours = static_cast<uint8_t>(hours);
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = 0;
        coord.tenths = static_cast<uint8_t>(tenths);
    }
    
    return ParseResult::Success;
}

/* ========================================================================
 * Declination Parsing
 * ======================================================================== */

ParseResult parse_dec_coordinate(
    const char* str,
    PrecisionMode mode,
    DECCoordinate& coord
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: sDD*MM:SS or sDD*MM
    // Check sign
    if (str[0] != '+' && str[0] != '-') {
        return ParseResult::ErrorInvalidFormat;
    }
    char sign = str[0];
    str++; // Skip sign
    
    // Find degree separator (*) 
    const char* star = find_char(str, '*');
    if (!star) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees
    uint32_t degrees;
    if (!parse_uint(str, star - str, degrees) || degrees > 90) {
        return ParseResult::ErrorOutOfRange;
    }
    
    const char* arcmin_start = star + 1;
    
    if (mode == PrecisionMode::High) {
        // High precision: sDD*MM:SS or sDD*MM'SS
        const char* sep = find_char(arcmin_start, ':');
        if (!sep) {
            sep = find_char(arcmin_start, '\'');  // Try apostrophe
        }
        
        if (sep) {
            // Parse arcminutes
            uint32_t arcminutes;
            if (!parse_uint(arcmin_start, sep - arcmin_start, arcminutes) || arcminutes >= 60) {
                return ParseResult::ErrorOutOfRange;
            }
            
            // Parse arcseconds
            uint32_t arcseconds;
            const char* arcsec_start = sep + 1;
            if (!parse_uint(arcsec_start, 2, arcseconds) || arcseconds >= 60) {
                return ParseResult::ErrorOutOfRange;
            }
            
            coord.sign = sign;
            coord.degrees = static_cast<uint8_t>(degrees);
            coord.arcminutes = static_cast<uint8_t>(arcminutes);
            coord.arcseconds = static_cast<uint8_t>(arcseconds);
        }
        else {
            // Maybe low precision format even in high mode
            return ParseResult::ErrorInvalidFormat;
        }
    }
    else {
        // Low precision: sDD*MM
        uint32_t arcminutes;
        if (!parse_uint(arcmin_start, 2, arcminutes) || arcminutes >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        coord.sign = sign;
        coord.degrees = static_cast<uint8_t>(degrees);
        coord.arcminutes = static_cast<uint8_t>(arcminutes);
        coord.arcseconds = 0;
    }
    
    return ParseResult::Success;
}

/* ========================================================================
 * Latitude Parsing
 * ======================================================================== */

ParseResult parse_latitude_coordinate(
    const char* str,
    LatitudeCoordinate& coord
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: sDD*MM
    if (str[0] != '+' && str[0] != '-') {
        return ParseResult::ErrorInvalidFormat;
    }
    char sign = str[0];
    str++; // Skip sign
    
    // Find degree separator
    const char* star = find_char(str, '*');
    if (!star) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees
    uint32_t degrees;
    if (!parse_uint(str, star - str, degrees) || degrees > 90) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse arcminutes
    const char* arcmin_start = star + 1;
    uint32_t arcminutes;
    if (!parse_uint(arcmin_start, 2, arcminutes) || arcminutes >= 60) {
        return ParseResult::ErrorOutOfRange;
    }
    
    coord.sign = sign;
    coord.degrees = static_cast<uint8_t>(degrees);
    coord.arcminutes = static_cast<uint8_t>(arcminutes);
    
    return ParseResult::Success;
}

/* ========================================================================
 * Longitude Parsing
 * ======================================================================== */

ParseResult parse_longitude_coordinate(
    const char* str,
    LongitudeCoordinate& coord
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: DDD*MM
    const char* star = find_char(str, '*');
    if (!star) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees (0-359)
    uint32_t degrees;
    if (!parse_uint(str, star - str, degrees) || degrees >= 360) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse arcminutes
    const char* arcmin_start = star + 1;
    uint32_t arcminutes;
    if (!parse_uint(arcmin_start, 2, arcminutes) || arcminutes >= 60) {
        return ParseResult::ErrorOutOfRange;
    }
    
    coord.degrees = static_cast<uint16_t>(degrees);
    coord.arcminutes = static_cast<uint8_t>(arcminutes);
    
    return ParseResult::Success;
}

/* ========================================================================
 * Time Parsing
 * ======================================================================== */

ParseResult parse_time_value(
    const char* str,
    TimeValue& time
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: HH:MM:SS
    const char* colon1 = find_char(str, ':');
    if (!colon1) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse hours
    uint32_t hours;
    if (!parse_uint(str, colon1 - str, hours) || hours >= 24) {
        return ParseResult::ErrorOutOfRange;
    }
    
    const char* minute_start = colon1 + 1;
    const char* colon2 = find_char(minute_start, ':');
    if (!colon2) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse minutes
    uint32_t minutes;
    if (!parse_uint(minute_start, colon2 - minute_start, minutes) || minutes >= 60) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse seconds
    uint32_t seconds;
    const char* second_start = colon2 + 1;
    if (!parse_uint(second_start, 2, seconds) || seconds >= 60) {
        return ParseResult::ErrorOutOfRange;
    }
    
    time.hours = static_cast<uint8_t>(hours);
    time.minutes = static_cast<uint8_t>(minutes);
    time.seconds = static_cast<uint8_t>(seconds);
    
    return ParseResult::Success;
}

/* ========================================================================
 * Date Parsing
 * ======================================================================== */

ParseResult parse_date_value(
    const char* str,
    DateValue& date
) noexcept
{
    if (!str) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: MM/DD/YY
    const char* slash1 = find_char(str, '/');
    if (!slash1) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse month
    uint32_t month;
    if (!parse_uint(str, slash1 - str, month) || month < 1 || month > 12) {
        return ParseResult::ErrorOutOfRange;
    }
    
    const char* day_start = slash1 + 1;
    const char* slash2 = find_char(day_start, '/');
    if (!slash2) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse day
    uint32_t day;
    if (!parse_uint(day_start, slash2 - day_start, day) || day < 1 || day > 31) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse year
    uint32_t year;
    const char* year_start = slash2 + 1;
    if (!parse_uint(year_start, 2, year) || year > 99) {
        return ParseResult::ErrorOutOfRange;
    }
    
    date.month = static_cast<uint8_t>(month);
    date.day = static_cast<uint8_t>(day);
    date.year = static_cast<uint8_t>(year);
    
    return ParseResult::Success;
}

} // namespace lx200

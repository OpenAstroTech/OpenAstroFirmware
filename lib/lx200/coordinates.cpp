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

/* ========================================================================
 * Right Ascension Parsing
 * ======================================================================== */

ParseResult parse_ra_coordinate(
    std::string_view str,
    PrecisionMode mode,
    RACoordinate& coord
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: HH:MM:SS (high) or HH:MM.T (low)
    size_t colon1_pos = str.find(':');
    if (colon1_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Hours must be exactly 2 digits
    if (colon1_pos != 2) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse hours
    uint32_t hours;
    if (!parse_uint(str.data(), colon1_pos, hours)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (hours >= 24) {
        return ParseResult::ErrorOutOfRange;
    }
    
    std::string_view minute_part = str.substr(colon1_pos + 1);
    
    if (mode == PrecisionMode::High) {
        // High precision: HH:MM:SS
        size_t colon2_pos = minute_part.find(':');
        if (colon2_pos == std::string_view::npos) {
            return ParseResult::ErrorInvalidFormat;
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_part.data(), colon2_pos, minutes)) {
            return ParseResult::ErrorInvalidFormat;
        }
        if (minutes >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        // Parse seconds (must be exactly 2 digits)
        std::string_view second_part = minute_part.substr(colon2_pos + 1);
        uint32_t seconds;
        if (!parse_uint(second_part.data(), 2, seconds)) {
            return ParseResult::ErrorInvalidFormat;
        }
        if (seconds >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        coord.hours = static_cast<uint8_t>(hours);
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = static_cast<uint8_t>(seconds);
    }
    else {
        // Low precision: HH:MM.T
        // Convert tenths of arcminutes to arcseconds: 0.1 arcmin = 6 arcsec
        size_t dot_pos = minute_part.find('.');
        if (dot_pos == std::string_view::npos) {
            return ParseResult::ErrorInvalidFormat;
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_part.data(), dot_pos, minutes)) {
            return ParseResult::ErrorInvalidFormat;
        }
        if (minutes >= 60) {
            return ParseResult::ErrorOutOfRange;
        }
        
        // Parse tenths (0-9)
        std::string_view tenth_part = minute_part.substr(dot_pos + 1);
        uint32_t tenths;
        if (!parse_uint(tenth_part.data(), 1, tenths)) {
            return ParseResult::ErrorInvalidFormat;
        }
        if (tenths >= 10) {
            return ParseResult::ErrorOutOfRange;
        }
        
        coord.hours = static_cast<uint8_t>(hours);
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = static_cast<uint8_t>(tenths * 6);  // Convert tenths to seconds
    }
    
    return ParseResult::Success;
}

/* ========================================================================
 * Declination Parsing
 * ======================================================================== */

ParseResult parse_dec_coordinate(
    std::string_view str,
    PrecisionMode mode,
    DECCoordinate& coord
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: sDD*MM:SS or sDD*MM
    // Check sign
    if (str[0] != '+' && str[0] != '-') {
        return ParseResult::ErrorInvalidFormat;
    }
    char sign = str[0];
    std::string_view str_nosign = str.substr(1); // Skip sign
    
    // Find degree separator (*) 
    size_t star_pos = str_nosign.find('*');
    if (star_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees
    uint32_t degrees;
    if (!parse_uint(str_nosign.data(), star_pos, degrees)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (degrees > 90) {
        return ParseResult::ErrorOutOfRange;
    }
    
    std::string_view arcmin_part = str_nosign.substr(star_pos + 1);
    
    if (mode == PrecisionMode::High) {
        // High precision: sDD*MM:SS or sDD*MM'SS
        size_t sep_pos = arcmin_part.find(':');
        if (sep_pos == std::string_view::npos) {
            sep_pos = arcmin_part.find('\'');  // Try apostrophe
        }
        
        if (sep_pos != std::string_view::npos) {
            // Parse arcminutes
            uint32_t arcminutes;
            if (!parse_uint(arcmin_part.data(), sep_pos, arcminutes)) {
                return ParseResult::ErrorInvalidFormat;
            }
            if (arcminutes >= 60) {
                return ParseResult::ErrorOutOfRange;
            }
            
            // Parse arcseconds
            std::string_view arcsec_part = arcmin_part.substr(sep_pos + 1);
            uint32_t arcseconds;
            if (!parse_uint(arcsec_part.data(), 2, arcseconds)) {
                return ParseResult::ErrorInvalidFormat;
            }
            if (arcseconds >= 60) {
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
        if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
            return ParseResult::ErrorInvalidFormat;
        }
        if (arcminutes >= 60) {
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
    std::string_view str,
    LatitudeCoordinate& coord
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: sDD*MM
    if (str[0] != '+' && str[0] != '-') {
        return ParseResult::ErrorInvalidFormat;
    }
    char sign = str[0];
    std::string_view str_nosign = str.substr(1); // Skip sign
    
    // Find degree separator
    size_t star_pos = str_nosign.find('*');
    if (star_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees
    uint32_t degrees;
    if (!parse_uint(str_nosign.data(), star_pos, degrees)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (degrees > 90) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse arcminutes
    std::string_view arcmin_part = str_nosign.substr(star_pos + 1);
    uint32_t arcminutes;
    if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (arcminutes >= 60) {
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
    std::string_view str,
    LongitudeCoordinate& coord
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: DDD*MM
    size_t star_pos = str.find('*');
    if (star_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse degrees (0-359)
    uint32_t degrees;
    if (!parse_uint(str.data(), star_pos, degrees)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (degrees >= 360) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse arcminutes
    std::string_view arcmin_part = str.substr(star_pos + 1);
    uint32_t arcminutes;
    if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (arcminutes >= 60) {
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
    std::string_view str,
    TimeValue& time
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: HH:MM:SS
    size_t colon1_pos = str.find(':');
    if (colon1_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse hours
    uint32_t hours;
    if (!parse_uint(str.data(), colon1_pos, hours)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (hours >= 24) {
        return ParseResult::ErrorOutOfRange;
    }
    
    std::string_view minute_part = str.substr(colon1_pos + 1);
    size_t colon2_pos = minute_part.find(':');
    if (colon2_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse minutes
    uint32_t minutes;
    if (!parse_uint(minute_part.data(), colon2_pos, minutes)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (minutes >= 60) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse seconds
    std::string_view second_part = minute_part.substr(colon2_pos + 1);
    uint32_t seconds;
    if (!parse_uint(second_part.data(), 2, seconds)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (seconds >= 60) {
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
    std::string_view str,
    DateValue& date
) noexcept
{
    if (str.empty()) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Format: MM/DD/YY
    size_t slash1_pos = str.find('/');
    if (slash1_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse month
    uint32_t month;
    if (!parse_uint(str.data(), slash1_pos, month)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (month < 1 || month > 12) {
        return ParseResult::ErrorOutOfRange;
    }
    
    std::string_view day_part = str.substr(slash1_pos + 1);
    size_t slash2_pos = day_part.find('/');
    if (slash2_pos == std::string_view::npos) {
        return ParseResult::ErrorInvalidFormat;
    }
    
    // Parse day
    uint32_t day;
    if (!parse_uint(day_part.data(), slash2_pos, day)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (day < 1 || day > 31) {
        return ParseResult::ErrorOutOfRange;
    }
    
    // Parse year
    std::string_view year_part = day_part.substr(slash2_pos + 1);
    uint32_t year;
    if (!parse_uint(year_part.data(), 2, year)) {
        return ParseResult::ErrorInvalidFormat;
    }
    if (year > 99) {
        return ParseResult::ErrorOutOfRange;
    }
    
    date.month = static_cast<uint8_t>(month);
    date.day = static_cast<uint8_t>(day);
    date.year = static_cast<uint8_t>(year);
    
    return ParseResult::Success;
}

} // namespace lx200

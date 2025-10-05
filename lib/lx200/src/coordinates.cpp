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
 * Right Ascension Parsing - Result<T> API
 * ======================================================================== */

Result<RACoordinate> parse_ra(std::string_view str, PrecisionMode mode) noexcept
{
    if (str.empty()) {
        return Err<RACoordinate>(ParseError::InvalidFormat);
    }
    
    // Format: HH:MM:SS (high) or HH:MM.T (low)
    size_t colon1_pos = str.find(':');
    if (colon1_pos == std::string_view::npos) {
        return Err<RACoordinate>(ParseError::InvalidFormat);
    }
    
    // Hours must be exactly 2 digits
    if (colon1_pos != 2) {
        return Err<RACoordinate>(ParseError::InvalidFormat);
    }
    
    // Parse hours
    uint32_t hours;
    if (!parse_uint(str.data(), colon1_pos, hours)) {
        return Err<RACoordinate>(ParseError::InvalidFormat);
    }
    if (hours >= 24) {
        return Err<RACoordinate>(ParseError::OutOfRange);
    }
    
    std::string_view minute_part = str.substr(colon1_pos + 1);
    
    RACoordinate coord{};
    coord.hours = static_cast<uint8_t>(hours);
    
    if (mode == PrecisionMode::High) {
        // High precision: HH:MM:SS
        size_t colon2_pos = minute_part.find(':');
        if (colon2_pos == std::string_view::npos) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_part.data(), colon2_pos, minutes)) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        if (minutes >= 60) {
            return Err<RACoordinate>(ParseError::OutOfRange);
        }
        
        // Parse seconds (must be exactly 2 digits, no trailing characters)
        std::string_view second_part = minute_part.substr(colon2_pos + 1);
        if (second_part.size() != 2) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        
        uint32_t seconds;
        if (!parse_uint(second_part.data(), 2, seconds)) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        if (seconds >= 60) {
            return Err<RACoordinate>(ParseError::OutOfRange);
        }
        
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = static_cast<uint8_t>(seconds);
    }
    else {
        // Low precision: HH:MM.T
        size_t dot_pos = minute_part.find('.');
        if (dot_pos == std::string_view::npos) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        
        // Parse minutes
        uint32_t minutes;
        if (!parse_uint(minute_part.data(), dot_pos, minutes)) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        if (minutes >= 60) {
            return Err<RACoordinate>(ParseError::OutOfRange);
        }
        
        // Parse tenths (must be exactly 1 digit, no trailing characters)
        std::string_view tenth_part = minute_part.substr(dot_pos + 1);
        if (tenth_part.size() != 1) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        
        uint32_t tenths;
        if (!parse_uint(tenth_part.data(), 1, tenths)) {
            return Err<RACoordinate>(ParseError::InvalidFormat);
        }
        if (tenths >= 10) {
            return Err<RACoordinate>(ParseError::OutOfRange);
        }
        
        coord.minutes = static_cast<uint8_t>(minutes);
        coord.seconds = static_cast<uint8_t>(tenths * 6);  // Convert tenths to seconds
    }
    
    return Ok(coord);
}

/* ========================================================================
 * Declination Parsing - Result<T> API
 * ======================================================================== */

Result<DECCoordinate> parse_dec(std::string_view str, PrecisionMode mode) noexcept
{
    if (str.empty()) {
        return Err<DECCoordinate>(ParseError::InvalidFormat);
    }
    
    // Must start with sign
    char sign = str[0];
    if (sign != '+' && sign != '-') {
        return Err<DECCoordinate>(ParseError::InvalidFormat);
    }
    
    // Find degree separator
    size_t deg_sep = str.find('*');
    if (deg_sep == std::string_view::npos) {
        return Err<DECCoordinate>(ParseError::InvalidFormat);
    }
    
    // Parse degrees (2 digits after sign)
    if (deg_sep != 3) {
        return Err<DECCoordinate>(ParseError::InvalidFormat);
    }
    
    uint32_t degrees;
    if (!parse_uint(str.data() + 1, 2, degrees)) {
        return Err<DECCoordinate>(ParseError::InvalidFormat);
    }
    if (degrees > 90) {
        return Err<DECCoordinate>(ParseError::OutOfRange);
    }
    
    std::string_view arcmin_part = str.substr(deg_sep + 1);
    
    DECCoordinate coord{};
    coord.sign = sign;
    coord.degrees = static_cast<uint8_t>(degrees);
    
    if (mode == PrecisionMode::High) {
        // High precision: sDD*MM:SS or sDD*MM'SS
        size_t sep_pos = arcmin_part.find_first_of(":'");
        if (sep_pos == std::string_view::npos) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        
        // Parse arcminutes
        uint32_t arcminutes;
        if (!parse_uint(arcmin_part.data(), sep_pos, arcminutes)) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        if (arcminutes >= 60) {
            return Err<DECCoordinate>(ParseError::OutOfRange);
        }
        
        // Parse arcseconds (must be exactly 2 digits)
        std::string_view arcsec_part = arcmin_part.substr(sep_pos + 1);
        if (arcsec_part.size() != 2) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        
        uint32_t arcseconds;
        if (!parse_uint(arcsec_part.data(), 2, arcseconds)) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        if (arcseconds >= 60) {
            return Err<DECCoordinate>(ParseError::OutOfRange);
        }
        
        // DEC 90° is only valid when arcminutes and arcseconds are zero
        if (degrees == 90 && (arcminutes > 0 || arcseconds > 0)) {
            return Err<DECCoordinate>(ParseError::OutOfRange);
        }
        
        coord.arcminutes = static_cast<uint8_t>(arcminutes);
        coord.arcseconds = static_cast<uint8_t>(arcseconds);
    }
    else {
        // Low precision: sDD*MM (must be exactly 2 digits)
        if (arcmin_part.size() != 2) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        
        uint32_t arcminutes;
        if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
            return Err<DECCoordinate>(ParseError::InvalidFormat);
        }
        if (arcminutes >= 60) {
            return Err<DECCoordinate>(ParseError::OutOfRange);
        }
        
        // DEC 90° is only valid when arcminutes are zero
        if (degrees == 90 && arcminutes > 0) {
            return Err<DECCoordinate>(ParseError::OutOfRange);
        }
        
        coord.arcminutes = static_cast<uint8_t>(arcminutes);
        coord.arcseconds = 0;
    }
    
    return Ok(coord);
}

/* ========================================================================
 * Latitude Parsing - Result<T> API
 * ======================================================================== */

Result<LatitudeCoordinate> parse_latitude(std::string_view str) noexcept
{
    if (str.empty()) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    // Must start with sign
    char sign = str[0];
    if (sign != '+' && sign != '-') {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    // Find degree separator
    size_t deg_sep = str.find('*');
    if (deg_sep == std::string_view::npos) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    // Parse degrees (2 digits after sign)
    if (deg_sep != 3) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    uint32_t degrees;
    if (!parse_uint(str.data() + 1, 2, degrees)) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    if (degrees > 90) {
        return Err<LatitudeCoordinate>(ParseError::OutOfRange);
    }
    
    // Parse arcminutes (must be exactly 2 digits)
    std::string_view arcmin_part = str.substr(deg_sep + 1);
    if (arcmin_part.size() != 2) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    uint32_t arcminutes;
    if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
        return Err<LatitudeCoordinate>(ParseError::InvalidFormat);
    }
    if (arcminutes >= 60) {
        return Err<LatitudeCoordinate>(ParseError::OutOfRange);
    }
    
    // Latitude 90° is only valid when arcminutes are zero
    if (degrees == 90 && arcminutes > 0) {
        return Err<LatitudeCoordinate>(ParseError::OutOfRange);
    }
    
    LatitudeCoordinate coord{};
    coord.sign = sign;
    coord.degrees = static_cast<uint8_t>(degrees);
    coord.arcminutes = static_cast<uint8_t>(arcminutes);
    
    return Ok(coord);
}

/* ========================================================================
 * Longitude Parsing - Result<T> API
 * ======================================================================== */

Result<LongitudeCoordinate> parse_longitude(std::string_view str) noexcept
{
    if (str.empty()) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    // Find degree separator
    size_t deg_sep = str.find('*');
    if (deg_sep == std::string_view::npos) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    // Parse degrees (3 digits)
    if (deg_sep != 3) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    uint32_t degrees;
    if (!parse_uint(str.data(), 3, degrees)) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    if (degrees >= 360) {
        return Err<LongitudeCoordinate>(ParseError::OutOfRange);
    }
    
    // Parse arcminutes (must be exactly 2 digits)
    std::string_view arcmin_part = str.substr(deg_sep + 1);
    if (arcmin_part.size() != 2) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    
    uint32_t arcminutes;
    if (!parse_uint(arcmin_part.data(), 2, arcminutes)) {
        return Err<LongitudeCoordinate>(ParseError::InvalidFormat);
    }
    if (arcminutes >= 60) {
        return Err<LongitudeCoordinate>(ParseError::OutOfRange);
    }
    
    LongitudeCoordinate coord{};
    coord.degrees = static_cast<uint16_t>(degrees);
    coord.arcminutes = static_cast<uint8_t>(arcminutes);
    
    return Ok(coord);
}

/* ========================================================================
 * Time Parsing - Result<T> API
 * ======================================================================== */

Result<TimeValue> parse_time(std::string_view str) noexcept
{
    if (str.empty()) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    // Format: HH:MM:SS
    size_t colon1_pos = str.find(':');
    if (colon1_pos == std::string_view::npos) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    // Parse hours (must be 2 digits)
    if (colon1_pos != 2) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    uint32_t hours;
    if (!parse_uint(str.data(), 2, hours)) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    if (hours >= 24) {
        return Err<TimeValue>(ParseError::OutOfRange);
    }
    
    std::string_view minute_part = str.substr(colon1_pos + 1);
    size_t colon2_pos = minute_part.find(':');
    if (colon2_pos == std::string_view::npos) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    // Parse minutes (must be 2 digits)
    if (colon2_pos != 2) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    uint32_t minutes;
    if (!parse_uint(minute_part.data(), 2, minutes)) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    if (minutes >= 60) {
        return Err<TimeValue>(ParseError::OutOfRange);
    }
    
    // Parse seconds (must be exactly 2 digits, no trailing characters)
    std::string_view second_part = minute_part.substr(colon2_pos + 1);
    if (second_part.size() != 2) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    
    uint32_t seconds;
    if (!parse_uint(second_part.data(), 2, seconds)) {
        return Err<TimeValue>(ParseError::InvalidFormat);
    }
    if (seconds >= 60) {
        return Err<TimeValue>(ParseError::OutOfRange);
    }
    
    TimeValue time{};
    time.hours = static_cast<uint8_t>(hours);
    time.minutes = static_cast<uint8_t>(minutes);
    time.seconds = static_cast<uint8_t>(seconds);
    
    return Ok(time);
}

/* ========================================================================
 * Date Parsing - Result<T> API
 * ======================================================================== */

Result<DateValue> parse_date(std::string_view str) noexcept
{
    if (str.empty()) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    // Format: MM/DD/YY
    size_t slash1_pos = str.find('/');
    if (slash1_pos == std::string_view::npos) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    // Parse month (must be 2 digits)
    if (slash1_pos != 2) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    uint32_t month;
    if (!parse_uint(str.data(), 2, month)) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    if (month < 1 || month > 12) {
        return Err<DateValue>(ParseError::OutOfRange);
    }
    
    std::string_view day_part = str.substr(slash1_pos + 1);
    size_t slash2_pos = day_part.find('/');
    if (slash2_pos == std::string_view::npos) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    // Parse day (must be 2 digits)
    if (slash2_pos != 2) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    uint32_t day;
    if (!parse_uint(day_part.data(), 2, day)) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    if (day < 1 || day > 31) {
        return Err<DateValue>(ParseError::OutOfRange);
    }
    
    // Parse year (must be exactly 2 digits, no trailing characters)
    std::string_view year_part = day_part.substr(slash2_pos + 1);
    if (year_part.size() != 2) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    
    uint32_t year;
    if (!parse_uint(year_part.data(), 2, year)) {
        return Err<DateValue>(ParseError::InvalidFormat);
    }
    if (year > 99) {
        return Err<DateValue>(ParseError::OutOfRange);
    }
    
    DateValue date{};
    date.month = static_cast<uint8_t>(month);
    date.day = static_cast<uint8_t>(day);
    date.year = static_cast<uint8_t>(year);
    
    return Ok(date);
}

} // namespace lx200

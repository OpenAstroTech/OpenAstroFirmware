# Contract: Coordinate Parsing Interface

**Feature**: 001-lx200-cpp20-rewrite  
**Contract Type**: Unit Interface Contract  
**Purpose**: Define the expected behavior of coordinate parsing functions

---

## Interface Definitions

```cpp
// RA Coordinate Parsing
std::optional<RACoordinate> parse_ra_coordinate(std::string_view str, PrecisionMode mode);

// DEC Coordinate Parsing  
std::optional<DECCoordinate> parse_dec_coordinate(std::string_view str, PrecisionMode mode);

// Latitude Coordinate Parsing
std::optional<LatitudeCoordinate> parse_latitude(std::string_view str);

// Longitude Coordinate Parsing
std::optional<LongitudeCoordinate> parse_longitude(std::string_view str);

// Time Parsing
std::optional<TimeValue> parse_time(std::string_view str);

// Date Parsing
std::optional<DateValue> parse_date(std::string_view str);
```

---

## RA Coordinate Contracts

### TC-RA-001: Parse Valid High Precision RA

**Given**: Input "12:34:56" in high precision mode

**When**: parse_ra_coordinate() is called

**Then**:
- Returns RACoordinate with hours=12, minutes=34, seconds=56
- Validation confirms values in range

**Test Code**:
```cpp
TEST(CoordinateContract, ParseRAHighPrecision) {
    auto ra = parse_ra_coordinate("12:34:56", PrecisionMode::High);
    
    ASSERT_TRUE(ra.has_value());
    ASSERT_EQ(ra->hours, 12);
    ASSERT_EQ(ra->minutes, 34);
    ASSERT_EQ(ra->seconds, 56);
}
```

---

### TC-RA-002: Parse Valid Low Precision RA

**Given**: Input "12:34.5" in low precision mode

**When**: parse_ra_coordinate() is called

**Then**:
- Returns RACoordinate with hours=12, minutes=34, seconds=30 (0.5 min = 30 sec)

**Test Code**:
```cpp
TEST(CoordinateContract, ParseRALowPrecision) {
    auto ra = parse_ra_coordinate("12:34.5", PrecisionMode::Low);
    
    ASSERT_TRUE(ra.has_value());
    ASSERT_EQ(ra->hours, 12);
    ASSERT_EQ(ra->minutes, 34);
    ASSERT_EQ(ra->seconds, 30);  // 0.5 * 60
}
```

---

### TC-RA-003: Reject Invalid RA Hours

**Given**: Input "24:00:00" (hours out of range)

**When**: parse_ra_coordinate() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectInvalidRAHours) {
    auto ra = parse_ra_coordinate("24:00:00", PrecisionMode::High);
    
    ASSERT_FALSE(ra.has_value());
}
```

---

### TC-RA-004: Reject Invalid RA Minutes

**Given**: Input "12:60:00" (minutes out of range)

**When**: parse_ra_coordinate() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectInvalidRAMinutes) {
    auto ra = parse_ra_coordinate("12:60:00", PrecisionMode::High);
    
    ASSERT_FALSE(ra.has_value());
}
```

---

### TC-RA-005: Reject Malformed RA String

**Given**: Input "12:34" (missing seconds in high precision)

**When**: parse_ra_coordinate() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectMalformedRA) {
    auto ra = parse_ra_coordinate("12:34", PrecisionMode::High);
    
    ASSERT_FALSE(ra.has_value());
}
```

---

## DEC Coordinate Contracts

### TC-DEC-001: Parse Valid Positive DEC

**Given**: Input "+45*30:15" in high precision mode

**When**: parse_dec_coordinate() is called

**Then**:
- Returns DECCoordinate with sign=+1, degrees=45, minutes=30, seconds=15

**Test Code**:
```cpp
TEST(CoordinateContract, ParseDECPositive) {
    auto dec = parse_dec_coordinate("+45*30:15", PrecisionMode::High);
    
    ASSERT_TRUE(dec.has_value());
    ASSERT_EQ(dec->sign, 1);
    ASSERT_EQ(dec->degrees, 45);
    ASSERT_EQ(dec->minutes, 30);
    ASSERT_EQ(dec->seconds, 15);
}
```

---

### TC-DEC-002: Parse Valid Negative DEC

**Given**: Input "-12*00:30" in high precision mode

**When**: parse_dec_coordinate() is called

**Then**:
- Returns DECCoordinate with sign=-1, degrees=12, minutes=0, seconds=30

**Test Code**:
```cpp
TEST(CoordinateContract, ParseDECNegative) {
    auto dec = parse_dec_coordinate("-12*00:30", PrecisionMode::High);
    
    ASSERT_TRUE(dec.has_value());
    ASSERT_EQ(dec->sign, -1);
    ASSERT_EQ(dec->degrees, 12);
    ASSERT_EQ(dec->minutes, 0);
    ASSERT_EQ(dec->seconds, 30);
}
```

---

### TC-DEC-003: Parse DEC with Apostrophe Separator

**Given**: Input "+45*30'15" (apostrophe instead of colon)

**When**: parse_dec_coordinate() is called

**Then**: Returns valid DECCoordinate (both separators supported)

**Test Code**:
```cpp
TEST(CoordinateContract, ParseDECApostrophe) {
    auto dec = parse_dec_coordinate("+45*30'15", PrecisionMode::High);
    
    ASSERT_TRUE(dec.has_value());
    ASSERT_EQ(dec->degrees, 45);
    ASSERT_EQ(dec->minutes, 30);
    ASSERT_EQ(dec->seconds, 15);
}
```

---

### TC-DEC-004: Reject DEC Out of Range

**Given**: Input "+90*30:00" (degrees > 90 not valid with minutes)

**When**: parse_dec_coordinate() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectDECOutOfRange) {
    auto dec = parse_dec_coordinate("+90*30:00", PrecisionMode::High);
    
    ASSERT_FALSE(dec.has_value());
}
```

---

### TC-DEC-005: Parse Low Precision DEC

**Given**: Input "+45*30" in low precision mode

**When**: parse_dec_coordinate() is called

**Then**: Returns DECCoordinate with seconds=0

**Test Code**:
```cpp
TEST(CoordinateContract, ParseDECLowPrecision) {
    auto dec = parse_dec_coordinate("+45*30", PrecisionMode::Low);
    
    ASSERT_TRUE(dec.has_value());
    ASSERT_EQ(dec->degrees, 45);
    ASSERT_EQ(dec->minutes, 30);
    ASSERT_EQ(dec->seconds, 0);
}
```

---

## Latitude Coordinate Contracts

### TC-LAT-001: Parse Valid Latitude

**Given**: Input "+45*30"

**When**: parse_latitude() is called

**Then**: Returns LatitudeCoordinate with sign=+1, degrees=45, minutes=30

**Test Code**:
```cpp
TEST(CoordinateContract, ParseLatitude) {
    auto lat = parse_latitude("+45*30");
    
    ASSERT_TRUE(lat.has_value());
    ASSERT_EQ(lat->sign, 1);
    ASSERT_EQ(lat->degrees, 45);
    ASSERT_EQ(lat->minutes, 30);
}
```

---

### TC-LAT-002: Reject Latitude Out of Range

**Given**: Input "+90*01" (exceeds 90 degrees)

**When**: parse_latitude() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectLatitudeOutOfRange) {
    auto lat = parse_latitude("+90*01");
    
    ASSERT_FALSE(lat.has_value());
}
```

---

## Longitude Coordinate Contracts

### TC-LON-001: Parse Valid Longitude

**Given**: Input "123*45"

**When**: parse_longitude() is called

**Then**: Returns LongitudeCoordinate with degrees=123, minutes=45

**Test Code**:
```cpp
TEST(CoordinateContract, ParseLongitude) {
    auto lon = parse_longitude("123*45");
    
    ASSERT_TRUE(lon.has_value());
    ASSERT_EQ(lon->degrees, 123);
    ASSERT_EQ(lon->minutes, 45);
}
```

---

### TC-LON-002: Reject Longitude Out of Range

**Given**: Input "360*00" (must be < 360)

**When**: parse_longitude() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectLongitudeOutOfRange) {
    auto lon = parse_longitude("360*00");
    
    ASSERT_FALSE(lon.has_value());
}
```

---

## Time Value Contracts

### TC-TIME-001: Parse Valid Time

**Given**: Input "14:30:45"

**When**: parse_time() is called

**Then**: Returns TimeValue with hours=14, minutes=30, seconds=45

**Test Code**:
```cpp
TEST(CoordinateContract, ParseTime) {
    auto time = parse_time("14:30:45");
    
    ASSERT_TRUE(time.has_value());
    ASSERT_EQ(time->hours, 14);
    ASSERT_EQ(time->minutes, 30);
    ASSERT_EQ(time->seconds, 45);
}
```

---

### TC-TIME-002: Reject Invalid Time

**Given**: Input "25:00:00" (hours > 23)

**When**: parse_time() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectInvalidTime) {
    auto time = parse_time("25:00:00");
    
    ASSERT_FALSE(time.has_value());
}
```

---

## Date Value Contracts

### TC-DATE-001: Parse Valid Date

**Given**: Input "10/04/25"

**When**: parse_date() is called

**Then**: Returns DateValue with month=10, day=4, year=25

**Test Code**:
```cpp
TEST(CoordinateContract, ParseDate) {
    auto date = parse_date("10/04/25");
    
    ASSERT_TRUE(date.has_value());
    ASSERT_EQ(date->month, 10);
    ASSERT_EQ(date->day, 4);
    ASSERT_EQ(date->year, 25);
}
```

---

### TC-DATE-002: Reject Invalid Month

**Given**: Input "13/01/25" (month > 12)

**When**: parse_date() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectInvalidMonth) {
    auto date = parse_date("13/01/25");
    
    ASSERT_FALSE(date.has_value());
}
```

---

### TC-DATE-003: Reject Invalid Day for Month

**Given**: Input "02/30/25" (February doesn't have 30 days)

**When**: parse_date() is called

**Then**: Returns std::nullopt

**Test Code**:
```cpp
TEST(CoordinateContract, RejectInvalidDay) {
    auto date = parse_date("02/30/25");
    
    ASSERT_FALSE(date.has_value());
}
```

---

## Contract Invariants

1. **Non-Throwing**: All functions MUST use std::optional, never throw exceptions
2. **Validation**: All returned coordinates MUST be in valid ranges
3. **Immutability**: Input strings MUST NOT be modified
4. **Determinism**: Same input MUST always produce same output
5. **String Safety**: MUST handle empty strings, null terminators correctly

---

## Performance Contracts

- **Parsing**: <5μs per coordinate (simple string scanning, no allocation)
- **Validation**: <1μs (constexpr range checks when possible)
- **Memory**: Zero dynamic allocation

---

## Error Handling Contracts

- **Invalid Format**: MUST return std::nullopt (not crash)
- **Out of Range**: MUST return std::nullopt after validation
- **Missing Data**: MUST return std::nullopt (not partial results)
- **Malformed Input**: MUST NOT corrupt process state

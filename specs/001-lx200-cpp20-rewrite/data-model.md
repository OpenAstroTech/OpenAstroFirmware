# Data Model: LX200 Protocol C++20 Rewrite

**Feature**: 001-lx200-cpp20-rewrite  
**Date**: 2025-10-04  
**Status**: Complete

## Overview

This document defines the data structures and entities for the LX200 protocol C++20 implementation. The design emphasizes simplicity, type safety, and zero-cost abstractions appropriate for embedded systems.

---

## Core Entities

### 1. CommandFamily (Enum)

**Purpose**: Identifies the category of an LX200 command based on its first character.

**Definition**:
```cpp
enum class CommandFamily : uint8_t {
    Alignment       = 'A',  // Alignment commands
    Reticle         = 'B',  // Reticle/accessory control
    Sync            = 'C',  // Sync control
    Distance        = 'D',  // Distance bars
    Focuser         = 'F',  // Focuser control
    GetInfo         = 'G',  // Get telescope information
    GPS             = 'g',  // GPS commands
    Home            = 'h',  // Home/park commands
    Hour            = 'H',  // Hour angle commands
    Initialize      = 'I',  // Initialize telescope
    Library         = 'L',  // Library commands
    Movement        = 'M',  // Movement commands
    Precision       = 'P',  // Precision toggle
    Rate            = 'R',  // Rate control
    SetInfo         = 'S',  // Set telescope information
    Tracking        = 'T',  // Tracking commands
    UserFormat      = 'U',  // User format control
    Extended        = 'X',  // Extended OAT commands
    Quit            = 'Q',  // Quit/stop commands
    Unknown         = 0     // Invalid/unrecognized
};
```

**Validation Rules**:
- Each valid command must map to a family
- Unknown characters map to CommandFamily::Unknown

**State Transitions**: N/A (enum is stateless)

---

### 2. PrecisionMode (Enum)

**Purpose**: Tracks whether coordinates should be formatted in high or low precision.

**Definition**:
```cpp
enum class PrecisionMode : uint8_t {
    High,  // HH:MM:SS format (default)
    Low    // HH:MM.T format
};
```

**Validation Rules**:
- Default is High precision
- Mode persists across commands until changed

**State Transitions**:
- `:P#` command toggles between High ↔ Low

---

### 3. ParseResult (Enum)

**Purpose**: Indicates the outcome of a parsing operation.

**Definition**:
```cpp
enum class ParseResult : uint8_t {
    Success,            // Command parsed successfully
    Incomplete,         // Waiting for more data
    InvalidPrefix,      // Missing ':' prefix
    InvalidTerminator,  // Missing '#' terminator
    InvalidCommand,     // Unrecognized command
    InvalidParameter,   // Parameter format error
    BufferOverflow,     // Command exceeds buffer size
    Error               // General parsing error
};
```

**Validation Rules**: N/A (result code)

---

### 4. RACoordinate (Struct)

**Purpose**: Represents Right Ascension coordinate.

**Definition**:
```cpp
struct RACoordinate {
    uint8_t hours;      // 0-23
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59 (0 in low precision mode)
    
    // Validation
    static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) {
        return h < 24 && m < 60 && s < 60;
    }
    
    // Construction with validation
    static std::optional<RACoordinate> create(uint8_t h, uint8_t m, uint8_t s);
};
```

**Validation Rules**:
- Hours: 0-23 (24-hour format, maps to 0-360 degrees)
- Minutes: 0-59
- Seconds: 0-59
- Total must represent valid RA (0h to 24h exclusive)

**Format Variants**:
- High precision: `HH:MM:SS` (e.g., "12:34:56")
- Low precision: `HH:MM.T` (e.g., "12:34.5", tenths of minute)

---

### 5. DECCoordinate (Struct)

**Purpose**: Represents Declination coordinate.

**Definition**:
```cpp
struct DECCoordinate {
    int8_t sign;        // +1 or -1
    uint8_t degrees;    // 0-89
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59 (0 in low precision mode)
    
    // Validation
    static constexpr bool is_valid(int8_t sgn, uint8_t d, uint8_t m, uint8_t s) {
        return (sgn == 1 || sgn == -1) && d <= 89 && m < 60 && s < 60;
    }
    
    // Construction with validation
    static std::optional<DECCoordinate> create(int8_t sgn, uint8_t d, uint8_t m, uint8_t s);
    
    // Convert to decimal degrees
    constexpr double to_degrees() const {
        return sign * (degrees + minutes/60.0 + seconds/3600.0);
    }
};
```

**Validation Rules**:
- Sign: + or - (stored as +1/-1)
- Degrees: 0-89 (90° not valid for declination with minutes/seconds)
- Minutes: 0-59
- Seconds: 0-59
- Range: -90° to +90° (pole limits)

**Format Variants**:
- High precision: `sDD*MM:SS` (e.g., "+45*30:15", "-12*00:30")
- Low precision: `sDD*MM` (e.g., "+45*30", "-12*00")
- Alternative: `sDD*MM'SS` (apostrophe instead of colon)

---

### 6. LatitudeCoordinate (Struct)

**Purpose**: Represents geographic latitude.

**Definition**:
```cpp
struct LatitudeCoordinate {
    int8_t sign;        // +1 (N) or -1 (S)
    uint8_t degrees;    // 0-89
    uint8_t minutes;    // 0-59
    
    static constexpr bool is_valid(int8_t sgn, uint8_t d, uint8_t m) {
        return (sgn == 1 || sgn == -1) && d <= 89 && m < 60;
    }
    
    static std::optional<LatitudeCoordinate> create(int8_t sgn, uint8_t d, uint8_t m);
};
```

**Validation Rules**: Same as DEC but no seconds field

**Format**: `sDD*MM` (e.g., "+45*30", "-12*00")

---

### 7. LongitudeCoordinate (Struct)

**Purpose**: Represents geographic longitude.

**Definition**:
```cpp
struct LongitudeCoordinate {
    int16_t degrees;    // 0-359 (degrees west from Greenwich)
    uint8_t minutes;    // 0-59
    
    static constexpr bool is_valid(int16_t d, uint8_t m) {
        return d >= 0 && d < 360 && m < 60;
    }
    
    static std::optional<LongitudeCoordinate> create(int16_t d, uint8_t m);
};
```

**Validation Rules**:
- Degrees: 0-359 (west from Greenwich)
- Minutes: 0-59

**Format**: `DDD*MM` (e.g., "123*45")

---

### 8. TimeValue (Struct)

**Purpose**: Represents time in HH:MM:SS format.

**Definition**:
```cpp
struct TimeValue {
    uint8_t hours;      // 0-23
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59
    
    static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) {
        return h < 24 && m < 60 && s < 60;
    }
    
    static std::optional<TimeValue> create(uint8_t h, uint8_t m, uint8_t s);
};
```

**Validation Rules**: Standard 24-hour time validation

**Format**: `HH:MM:SS` (e.g., "14:30:45")

---

### 9. DateValue (Struct)

**Purpose**: Represents date in MM/DD/YY format.

**Definition**:
```cpp
struct DateValue {
    uint8_t month;      // 1-12
    uint8_t day;        // 1-31
    uint8_t year;       // 0-99 (2000-2099)
    
    static bool is_valid(uint8_t m, uint8_t d, uint8_t y);
    static std::optional<DateValue> create(uint8_t m, uint8_t d, uint8_t y);
};
```

**Validation Rules**:
- Month: 1-12
- Day: 1-31 (adjusted for month/leap year)
- Year: 0-99 (interpreted as 2000-2099)

**Format**: `MM/DD/YY` (e.g., "10/04/25")

---

### 10. Command (Struct)

**Purpose**: Represents a fully parsed LX200 command.

**Definition**:
```cpp
struct Command {
    CommandFamily family;
    std::string_view name;        // Full command string (e.g., "GR")
    std::string_view parameters;  // Parameter substring (may be empty)
    
    // Helper methods
    bool has_parameters() const { return !parameters.empty(); }
    char first_char() const { return name.empty() ? '\0' : name[0]; }
};
```

**Relationships**:
- Contains CommandFamily enum
- References command buffer via string_view (non-owning)

**Validation Rules**:
- name must not be empty
- family must be valid (not Unknown for valid commands)

---

### 11. ParserState (Class)

**Purpose**: Maintains state for incremental command parsing.

**Definition**:
```cpp
class ParserState {
public:
    ParserState();
    
    // State management
    void reset();
    ParseResult feed_character(char c);
    std::optional<Command> get_command();
    
    // Precision mode
    PrecisionMode get_precision() const { return precision_; }
    void set_precision(PrecisionMode mode) { precision_ = mode; }
    
private:
    static constexpr size_t MAX_COMMAND_LENGTH = 32;
    std::array<char, MAX_COMMAND_LENGTH> buffer_;
    size_t buffer_length_{0};
    bool command_complete_{false};
    PrecisionMode precision_{PrecisionMode::High};
};
```

**State Transitions**:
1. **Empty** → (feed ':') → **Accumulating**
2. **Accumulating** → (feed '#') → **Complete**
3. **Complete** → (get_command) → **Empty**
4. **Any** → (reset) → **Empty**
5. **Accumulating** → (overflow) → **Error**

**Relationships**:
- Owns command buffer
- Maintains PrecisionMode state
- Produces Command objects

---

## Data Flow

```
Serial Input Stream
       ↓
ParserState::feed_character()
       ↓
   [buffer accumulation]
       ↓
ParserState::get_command()
       ↓
    Command struct
       ↓
Command execution layer (Mount controller)
```

---

## Memory Management

All entities use:
- **Stack allocation**: Structs are small, fixed-size
- **No dynamic allocation**: Fixed buffers in ParserState
- **Non-owning references**: string_view for parsed substrings
- **RAII**: ParserState manages buffer lifecycle

**Memory Footprint Estimate**:
- ParserState: ~40 bytes (32-byte buffer + 8 bytes state)
- Command: ~24 bytes (2 string_views + enum)
- Coordinate structs: 3-4 bytes each
- Total parser overhead: <100 bytes

---

## Concurrency Considerations

- **Single-threaded**: Parser assumes single-threaded access
- **Asynchronous design**: Command processing is async, but parser itself is synchronous
- **No shared mutable state**: Each parser instance is independent
- **Thread-safety**: Not required (Zephyr UART callback runs in single context)

---

## Extensibility

**Adding New Command Families**:
1. Add enum value to CommandFamily
2. Update family identification logic
3. Add tests for new family

**Adding New Coordinate Types**:
1. Define new struct following existing pattern
2. Add static validation method
3. Add parsing function
4. Add tests

**Adding New Commands**:
- No data model changes required (commands are identified by string)
- Execution logic added in Mount controller layer

---

## Summary

The data model provides:
- ✅ Type-safe coordinate representations with compile-time validation
- ✅ Clear parsing state management with RAII
- ✅ Zero-cost abstractions (constexpr, string_view)
- ✅ No dynamic allocation
- ✅ Simple, testable design
- ✅ Easy to extend

All entities are designed for embedded constraints while leveraging modern C++20 features for safety and maintainability.

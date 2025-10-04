# Research: LX200 Protocol C++20 Rewrite

**Feature**: 001-lx200-cpp20-rewrite  
**Date**: 2025-10-04  
**Status**: Complete

## Overview

This document consolidates research findings for rewriting the LX200 telescope protocol parser from C to C++20. The focus is on identifying best practices for embedded C++20, avoiding overengineering, and ensuring the implementation remains simple and maintainable.

---

## Research Areas

### 1. C++20 Features Appropriate for Embedded Systems

**Decision**: Use constrained subset of C++20 features

**Rationale**:
- **string_view**: Essential for zero-copy parsing. Avoids string allocations when extracting substrings from command buffer.
- **constexpr**: Enables compile-time validation of protocol constants (command lengths, format strings).
- **Concepts**: Documents type requirements for template parameters (if used for validators).
- **std::optional**: Perfect for representing parse results that may or may not have values (better than nullable pointers).
- **Structured bindings**: Improves readability when returning multiple values from parsing functions.

**Avoid**:
- **std::expected** (C++23): Not available yet, use custom result type or std::optional
- **Exceptions**: Disabled in embedded context for deterministic control flow
- **RTTI**: Disabled to save memory
- **std::string dynamic allocation**: Use fixed-size buffers + string_view
- **std::vector/map**: Use compile-time arrays or fixed-size containers

**Alternatives Considered**: 
- Plain C++11: Rejected - missing string_view, constexpr enhancements
- C++17: Acceptable fallback, but C++20 provides better ergonomics
- C++23: Too new, not well-supported in embedded toolchains

**References**:
- Zephyr C++ support: C++20 with GCC 12+
- Embedded C++ guidelines: Avoid dynamic allocation, exceptions, RTTI

---

### 2. String Parsing Patterns Without Allocation

**Decision**: Use string_view for parsing, fixed buffers for storage

**Rationale**:
- Command buffer is fixed-size (LX200_MAX_COMMAND_LENGTH)
- Parsing extracts substrings as string_view (non-owning)
- Only allocate storage for parsed results (on stack or in result struct)
- Parser state maintains ring buffer for partial commands

**Pattern**:
```cpp
// Pseudo-code example (not actual implementation)
struct ParseResult {
    std::string_view command;      // Points into buffer
    std::string_view parameters;   // Points into buffer
    CommandFamily family;
    bool complete;
};

std::optional<ParseResult> parse_command(std::string_view buffer);
```

**Alternatives Considered**:
- C-style char* + length: Works but less safe
- std::string: Rejected - requires dynamic allocation
- Custom string class: Rejected - unnecessary complexity

---

### 3. Error Handling Without Exceptions

**Decision**: Use std::optional and result enums

**Rationale**:
- std::optional<T> for operations that may fail (returns empty on error)
- Result enums (ParseResult, ValidationError) for detailed error reporting
- Logging framework for diagnostic information
- No exceptions = predictable control flow + smaller binary size

**Pattern**:
```cpp
// Pseudo-code example
enum class ParseError {
    InvalidPrefix,
    InvalidTerminator,
    BufferOverflow,
    // ...
};

struct CommandResult {
    Command command;
    ParseError error;
    bool success;
};

CommandResult parse_command(std::string_view input);
```

**Alternatives Considered**:
- Exceptions: Rejected - disabled in embedded builds
- Return codes: Less type-safe, but acceptable
- std::expected (C++23): Not available yet

---

### 4. Coordinate Validation Approach

**Decision**: Separate validation functions with constexpr range checking

**Rationale**:
- Each coordinate type (RA, DEC, Lat, Lon, Time) has specific validation rules
- Validation separated from parsing for independent testing
- Constexpr range checking enables compile-time verification where possible
- Clear error reporting for out-of-range values

**Pattern**:
```cpp
// Pseudo-code example
struct RACoordinate {
    uint8_t hours;    // 0-23
    uint8_t minutes;  // 0-59
    uint8_t seconds;  // 0-59
    
    static constexpr bool is_valid(uint8_t h, uint8_t m, uint8_t s) {
        return h < 24 && m < 60 && s < 60;
    }
};

std::optional<RACoordinate> parse_ra_coordinate(std::string_view str);
```

**Alternatives Considered**:
- Runtime validation only: Less safe
- Throwing validators: Rejected - no exceptions
- Single validation function: Less testable

---

### 5. Parser State Management

**Decision**: Stateful parser class with RAII

**Rationale**:
- Parser maintains buffer for partial commands
- RAII ensures buffer cleanup
- State includes: current buffer, precision mode, completion flag
- Reset method for error recovery

**Pattern**:
```cpp
// Pseudo-code example
class LX200Parser {
public:
    LX200Parser();  // Initialize state
    ~LX200Parser() = default;  // RAII cleanup
    
    void reset();
    bool feed_character(char c);
    std::optional<Command> get_command();
    
private:
    std::array<char, MAX_CMD_LEN> buffer_;
    size_t buffer_len_{0};
    PrecisionMode precision_{PrecisionMode::High};
    bool command_complete_{false};
};
```

**Alternatives Considered**:
- Stateless parser: Rejected - doesn't handle partial commands
- C-style struct with functions: Works but less ergonomic
- Global state: Rejected - not thread-safe or testable

---

### 6. Command Family Identification

**Decision**: Compile-time command table with constexpr lookup

**Rationale**:
- Command families are fixed at compile time
- Constexpr lookup enables zero-cost abstraction
- Table-driven design separates command definitions from parsing logic
- Easy to extend with new commands

**Pattern**:
```cpp
// Pseudo-code example
enum class CommandFamily {
    Alignment,    // A
    Reticle,      // B
    Sync,         // C
    // ...
};

constexpr CommandFamily identify_family(char first_char) {
    // Compile-time lookup table
    switch(first_char) {
        case 'A': return CommandFamily::Alignment;
        case 'B': return CommandFamily::Reticle;
        // ...
    }
}
```

**Alternatives Considered**:
- Runtime string comparison: Slower, but simpler
- Hash table: Rejected - unnecessary for small command set
- Regex: Rejected - overkill and not available

---

### 7. Integration with Mount Controller

**Decision**: Clean C++ interface callable from Mount class

**Rationale**:
- Mount class is already C++20
- Direct C++ interface (no C wrapper needed)
- Parser provides command objects, Mount executes them
- Loose coupling through interface design

**Pattern**:
```cpp
// Pseudo-code example
// In Mount.cpp:
LX200Parser parser_;

void Mount::handle_serial_data(std::string_view data) {
    for (char c : data) {
        if (parser_.feed_character(c)) {
            if (auto cmd = parser_.get_command()) {
                execute_command(*cmd);
            }
        }
    }
}
```

**Alternatives Considered**:
- C interface with extern "C": Unnecessary complexity
- Callback-based: More complex state management
- Message queue: Overkill for this use case

---

### 8. Testing Strategy for C++20 Code

**Decision**: Zephyr Twister with native_sim, test-first approach

**Rationale**:
- Tests written before implementation (TDD)
- Zephyr Twister provides test framework
- native_sim allows PC-based testing
- Unit tests: individual parsing functions
- Integration tests: complete command sequences

**Test Categories**:
1. **Parser unit tests**: Command identification, buffer management, state transitions
2. **Coordinate parsing tests**: Each coordinate type, range validation, format variants
3. **Command tests**: Each command family, parameter extraction
4. **Integration tests**: Complete command sequences, error recovery, protocol compliance
5. **Performance tests**: Command processing latency, memory usage

**Alternatives Considered**:
- GoogleTest: Not integrated with Zephyr
- Custom test framework: Reinventing the wheel
- Manual testing only: Insufficient for TDD

---

### 9. Migration Strategy

**Decision**: Delete C code first, implement C++20 test-first

**Rationale**:
- Clean slate prevents mixing old/new code
- Forces complete redesign rather than incremental C-to-C++ translation
- Tests specify desired behavior for new implementation
- No compatibility burden from old C API

**Migration Steps**:
1. Delete `lib/lx200/lx200.c`, `include/lx200/lx200.h`
2. Write comprehensive test suite (based on old tests + new requirements)
3. Implement C++20 parser to make tests pass
4. Update Mount class to use new C++ interface
5. Verify protocol compliance with ASCOM/INDI integration tests

**Alternatives Considered**:
- Incremental rewrite: Risk of mixed C/C++ code
- Side-by-side implementation: Confusion and duplication
- Automatic C-to-C++ translation: Doesn't improve design

---

### 10. Build System Changes

**Decision**: Update CMakeLists.txt to compile C++ instead of C

**Rationale**:
- Zephyr CMake system supports C++ sources
- Change file extensions .c → .cpp
- Enable C++20 standard in CMakeLists.txt
- Update test build configuration

**Required Changes**:
- `lib/lx200/CMakeLists.txt`: Add C++ sources, set C++20 standard
- `tests/lib/lx200/CMakeLists.txt`: Update test sources to .cpp
- Verify C++ standard flags in project configuration

**Alternatives Considered**:
- Mixed C/C++ build: Unnecessary complexity
- Separate C++ library: Over-engineered

---

## Summary of Decisions

| Area | Decision | Key Benefit |
|------|----------|-------------|
| C++20 Features | constrained subset (string_view, constexpr, optional, concepts) | Safe for embedded, avoids allocation |
| String Parsing | string_view + fixed buffers | Zero-copy parsing |
| Error Handling | std::optional + result enums | No exceptions, type-safe |
| Validation | Separate constexpr validators | Testable, compile-time checking |
| Parser State | Stateful class with RAII | Clean resource management |
| Command Lookup | Compile-time constexpr table | Zero-cost abstraction |
| Integration | Direct C++ interface | Simple, no wrappers needed |
| Testing | Zephyr Twister + TDD | Comprehensive coverage |
| Migration | Delete-first, test-first | Clean design, no legacy burden |
| Build System | Update CMakeLists for C++ | Standard Zephyr C++ support |

---

## Open Questions

None - all technical decisions resolved in clarifications section of spec.md.

---

## Conclusion

The research confirms that a C++20 rewrite is feasible and beneficial:
- Modern C++ features improve safety without adding complexity
- Embedded constraints are respected (no exceptions, RTTI, dynamic allocation)
- Clean design emerges from test-first approach
- Integration with existing C++20 Mount code is straightforward
- Zephyr toolchain supports C++20 with appropriate compiler flags

The implementation can proceed to Phase 1 (Design & Contracts) with confidence.

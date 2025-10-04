# Quickstart: LX200 Protocol C++20 Rewrite

**Feature**: 001-lx200-cpp20-rewrite  
**Date**: 2025-10-04  
**Purpose**: Validate the LX200 C++20 implementation works correctly

## Prerequisites

- Zephyr development environment set up
- West workspace initialized
- Python virtual environment activated
- Branch: `001-lx200-cpp20-rewrite`

## Quick Validation Steps

### 1. Build for Native Simulation

```bash
# From repository root
cd app
west build -b native_sim -t pristine
west build -b native_sim
```

**Expected**: Clean build with no errors

### 2. Run Unit Tests

```bash
# From repository root
west twister -T tests/lib/lx200/ -p native_sim
```

**Expected**: All tests pass
- Parser initialization tests
- Command parsing tests
- Coordinate parsing tests
- Command family identification tests
- Integration tests

### 3. Verify Command Parsing

**Test**: Parser handles basic GET commands

```bash
# Run specific test scenario
west twister -T tests/lib/lx200/ -p native_sim -s test_parser_get_commands
```

**Expected Output**:
```
INFO  - Test execution successful
PASSED - tests/lib/lx200/test_parser.cpp:test_parse_get_ra_command
PASSED - tests/lib/lx200/test_parser.cpp:test_parse_get_dec_command
```

### 4. Verify Coordinate Parsing

**Test**: Coordinate parsers handle valid and invalid inputs

```bash
west twister -T tests/lib/lx200/ -p native_sim -s test_coordinates
```

**Expected Output**:
```
PASSED - tests/lib/lx200/test_coordinates.cpp:test_parse_ra_valid
PASSED - tests/lib/lx200/test_coordinates.cpp:test_parse_ra_invalid_hours
PASSED - tests/lib/lx200/test_coordinates.cpp:test_parse_dec_valid
PASSED - tests/lib/lx200/test_coordinates.cpp:test_parse_dec_out_of_range
```

### 5. Integration Test: Command Sequences

**Test**: Complete command sequences work end-to-end

```bash
west twister -T tests/lib/lx200/ -p native_sim -s test_integration
```

**Test Scenarios**:
1. Send `:GR#` → Expect RA response in `HH:MM:SS#` format
2. Send `:Gd#` → Expect DEC response in `sDD*MM:SS#` format
3. Send `:P#` → Toggle precision mode
4. Send `:GR#` again → Expect RA in `HH:MM.T#` format (low precision)
5. Send invalid command → Expect error response

**Expected**: All scenarios pass with correct responses

### 6. Performance Check

```bash
# Run performance benchmarks
west twister -T tests/lib/lx200/ -p native_sim -s test_performance
```

**Expected**:
- Command parsing: <10ms per command
- Coordinate parsing: <5ms per coordinate
- Parser state operations: <1ms

### 7. Build for Target Hardware

```bash
cd app
west build -b robin_nano -t pristine
west build -b robin_nano
```

**Expected**: Clean build for MKS Robin Nano target

### 8. Flash and Manual Test (Optional)

If hardware available:

```bash
west flash
```

Then connect via serial terminal (115200 baud):
```
$ minicom -D /dev/ttyUSB0 -b 115200

# Type commands (should echo responses):
:GR#     → 12:34:56#
:Gd#     → +45*30:15#
:P#      → (no response, mode toggled)
:GR#     → 12:34.5#
```

## Validation Checklist

- [ ] Native simulation build succeeds
- [ ] All unit tests pass
- [ ] Parser tests pass (command identification, buffering)
- [ ] Coordinate tests pass (all formats, validation)
- [ ] Integration tests pass (command sequences)
- [ ] Performance benchmarks meet requirements (<100ms response)
- [ ] Hardware build succeeds
- [ ] Manual serial test successful (if hardware available)

## Troubleshooting

**Build Failures:**
- Verify C++20 enabled in CMakeLists.txt
- Check compiler version supports C++20
- Ensure all .c files renamed to .cpp

**Test Failures:**
- Check test output for specific assertion failures
- Verify test data uses correct LX200 protocol formats
- Review parser state management logic

**Performance Issues:**
- Profile with `west build -b native_sim -- -DCONFIG_DEBUG=y`
- Check for unexpected allocations
- Verify constexpr optimizations applied

## Success Criteria

✅ **Feature is ready when:**
1. All tests pass on native_sim
2. All tests pass on target hardware (robin_nano)
3. Performance benchmarks meet requirements
4. Integration with Mount controller verified
5. No regressions in existing functionality

## Next Steps After Validation

1. Update main application to use new C++ parser
2. Test with ASCOM/INDI integration
3. Performance profiling on hardware
4. Documentation updates
5. Code review and merge to main

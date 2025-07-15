# LX200 Library Test Suite

This directory contains comprehensive tests for the LX200 telescope protocol library.

## Test Structure

### `src/main.c`
Contains the main test suite covering:

- **Parser Initialization and State Tests**: Testing parser initialization, reset, and precision mode management
- **Command Parsing Tests**: Testing various LX200 command formats and parameter extraction
- **Command Family and Utility Tests**: Testing command classification and utility functions
- **Complex Command Parsing Tests**: Testing edge cases and complex command scenarios
- **Error Handling Tests**: Testing malformed commands and error conditions

### `src/test_coordinates.c`
Contains test specifications for coordinate parsing functions that are currently unimplemented:

- **Coordinate Parsing Tests**: RA, Dec, Alt/Az, longitude/latitude parsing
- **Time and Date Parsing Tests**: Time and date format parsing
- **Rate Parsing Tests**: Tracking and slew rate parsing
- **Formatting Tests**: Coordinate and time formatting functions
- **Validation Tests**: Input validation functions

## Test Categories

### Implemented Functions (Passing Tests)
These tests verify the currently working functionality:

- ✅ Parser state management
- ✅ Command string parsing
- ✅ Command family identification
- ✅ Parameter extraction
- ✅ Error handling for malformed commands
- ✅ Buffer management and overflow protection

### Unimplemented Functions (Specification Tests)
These tests currently fail as expected but serve as specifications for future implementation:

- ⏳ Coordinate parsing (RA, Dec, Alt/Az)
- ⏳ Geographic coordinate parsing (longitude, latitude)
- ⏳ Time and date parsing
- ⏳ Rate parsing (tracking, slew)
- ⏳ Coordinate and time formatting
- ⏳ Input validation functions

## Running the Tests

### Prerequisites
- Zephyr RTOS development environment
- LX200 library properly configured in the build system
- Zephyr environment sourced (`source ~/zephyrproject/zephyr/zephyr-env.sh`)

### Build and Run with Twister

The recommended way to run these tests is using Zephyr's Twister test framework:

```bash
# From the test directory
cd tests/lib/lx200
west twister -T . -p native_sim

# Or use the provided script
./run_tests.sh

# Run with verbose output
west twister -T . -p native_sim --verbose

# Run with coverage analysis
west twister -T . -p native_sim --coverage

# Run on different platforms
west twister -T . -p qemu_x86
west twister -T . -p qemu_cortex_m3

# Run only LX200 tagged tests (useful in larger projects)
west twister -T . --tag lx200
```

### Alternative: Manual Build and Run
```bash
# From the OpenAstroFirmware root directory
west build -p auto -b native_sim tests/lib/lx200
west build -t run
```

### Expected Output
The test suite will show:
- All parser and utility tests passing (green)
- All coordinate/formatting tests failing as expected (showing "Function not implemented yet")

## Test Coverage

### Currently Tested Functions
- `lx200_parser_init()`
- `lx200_parser_reset()`
- `lx200_parser_add_data()`
- `lx200_parse_command()`
- `lx200_parse_command_string()`
- `lx200_get_command_family()`
- `lx200_command_has_parameter()`
- `lx200_get_parameter_format()`
- `lx200_parse_result_to_string()`
- `lx200_set_precision_mode()`
- `lx200_get_precision_mode()`

### Functions Needing Implementation
- All coordinate parsing functions (`lx200_parse_*_coordinate()`)
- All formatting functions (`lx200_format_*()`)
- All validation functions (`lx200_validate_*()`)
- Time/date/rate parsing functions

## Test Scenarios Covered

1. **Valid Commands**: Standard LX200 commands with and without parameters
2. **Invalid Commands**: Malformed commands, wrong prefixes/terminators
3. **Boundary Conditions**: Maximum length commands, empty inputs
4. **Error Handling**: NULL pointers, buffer overflows
5. **State Management**: Parser state transitions
6. **Parameter Extraction**: Various parameter formats and special characters

## Adding New Tests

When implementing new functions:

1. Move the corresponding test from `test_coordinates.c` to `main.c`
2. Update the test to verify actual functionality instead of "not implemented"
3. Add additional test cases for edge cases and error conditions
4. Update this README to reflect the new test coverage

## LX200 Protocol Coverage

The tests cover the major LX200 command families:

- **A**: Alignment commands
- **B**: Reticle/accessory control
- **C**: Sync control
- **D**: Distance bars
- **F**: Focuser control
- **G**: Get telescope information
- **g**: GPS/magnetometer commands
- **H**: Time format command
- **I**: Initialize telescope
- **L**: Object library commands
- **M**: Movement commands
- **P**: High precision toggle
- **Q**: Movement stop commands
- **R**: Slew rate commands
- **S**: Set telescope parameters
- **T**: Tracking commands
- **U**: Precision toggle

## Known Limitations

1. Some complex parameter formats are simplified in the current implementation
2. Coordinate validation rules need to be implemented
3. Time zone handling is not yet implemented
4. Some LX200GPS extensions are not covered

## Future Enhancements

1. Add performance benchmarking tests
2. Add stress testing with rapid command sequences
3. Add tests for concurrent parser usage
4. Add integration tests with mock telescope hardware
5. Add fuzzing tests for robustness

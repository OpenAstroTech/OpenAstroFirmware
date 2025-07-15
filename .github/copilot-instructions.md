````instructions
# OpenAstroFirmware AI Agent Instructions

This is firmware for DIY astronomical telescope mounts, built on Zephyr RTOS. It's a modern C++20 rewrite of the successful OpenAstroTracker v1 firmware, emphasizing maintainability and advanced features.

## Architecture Overview

**Build System**: Zephyr RTOS v4.1.0 with west build tool
**Target Hardware**: MKS Robin Nano (STM32F407-based) boards  
**Languages**: C++20 primary, with C for low-level protocol implementations
**Structure**: Dual-app architecture - `app/` (main firmware) + `lib/` (reusable components)

Key directories:
- `app/` - Main Zephyr application with mount control logic
- `lib/lx200/` - LX200 telescope protocol implementation (C)
- `boards/mks/robin_nano/` - Custom board definition and device tree  
- `tests/` - Comprehensive test suites using Zephyr Twister
- `zephyr/module.yml` - Zephyr module configuration

## Development Workflow

### Building & Testing
```bash
west build -b robin_nano                    # Standard build
west build -b robin_nano -- -DEXTRA_CONF_FILE=debug.conf  # Debug build
west twister -T . -p robin_nano             # Run all tests
west twister -T tests/lib/lx200/             # Run specific test suite
```

### Configuration Layers
- **Kconfig**: `prj.conf` (base) + `debug.conf` (debug overlay)
- **Device Tree**: Custom STM32F407 board in `boards/mks/robin_nano/`
- **Build Configs**: Use `-DEXTRA_CONF_FILE=debug.conf` for development

## Code Patterns & Conventions

### Language Split Pattern
**C++20 Application Layer** (`app/src/`):
```cpp
#include <mount/Mount.hpp>
Mount mount;  // Main astronomical mount controller
```

**C Protocol Layer** (`lib/lx200/`):
```c
lx200_parser_state_t parser_state;
lx200_parse_command_string(cmd_string, &command);
```

### Logging Infrastructure
Every module uses Zephyr structured logging:
```cpp
LOG_MODULE_REGISTER(ModuleName, CONFIG_APP_LOG_LEVEL);
// Available: LOG_DBG(), LOG_INF(), LOG_WRN(), LOG_ERR()
```

### LX200 Protocol Implementation
Commands follow `:COMMAND[params]#` format. Current implementation status:
- ✅ **Parser Framework**: Command parsing, family classification
- ⏳ **Coordinate Functions**: All `lx200_parse_*_coordinate()` are stubs
- ⏳ **Formatting Functions**: All `lx200_format_*()` are stubs

Example of incomplete implementation pattern:
```c
lx200_parse_result_t lx200_parse_ra_coordinate(const char *str, lx200_coordinate_t *coord) {
    LOG_WRN("Function not implemented yet");
    return LX200_PARSE_ERROR;  // All coordinate functions are stubs
}
```

## Testing Strategy

### Test-Driven Development Pattern
Tests serve as specifications for unimplemented features in `tests/lib/lx200/`:

**Working Tests** (verify current functionality):
- Parser state management
- Command family identification (A,B,C,D,F,G,g,H,I,L,M,P,Q,R,S,T,U)
- Basic command/parameter separation

**Specification Tests** (currently failing, define requirements):
- Coordinate parsing (RA: `HH:MM:SS`, Dec: `sDD*MM:SS`)
- Time/date parsing (`HH:MM:SS`, `MM/DD/YY`)
- Response formatting functions

Run with: `west twister -T tests/lib/lx200/`

### Configuration Testing
Two build configurations tested via `sample.yaml`:
- `app.default`: Production configuration
- `app.debug`: Debug configuration with `CONFIG_APP_LOG_LEVEL_DBG=y`

## Project-Specific Context

### Astronomical Domain Knowledge
- **Coordinate Systems**: RA/Dec (celestial), Alt/Az (horizontal)
- **Precision Modes**: High (`HH:MM:SS`) vs Low (`HH:MM.T`) coordinate formats
- **LX200 Compatibility**: Industry standard for telescope communication
- **Mount Types**: Equatorial tracking, Alt-azimuth positioning

### Critical Implementation Details
**Parameter Detection Logic** (needs improvement):
```c
// Current: oversimplified - only checks 'S' commands
bool lx200_command_has_parameter(const char *command) {
    return (command[0] == 'S');  // Missing many parameter commands
}
```

**Command Parsing Edge Cases**:
- Special commands: `B+`, `B-`, `F+`, `F-`, `T+`, `T-`
- Rate commands: `R0` through `R9`
- Library commands and GPS commands also take parameters

### Development Status
Early development phase. Focus on:
1. Implementing coordinate parsing stubs in `lib/lx200/`
2. Following existing test specifications in `tests/lib/lx200/`
3. Using C for protocol layer, C++20 for application layer
4. Maintaining Zephyr module structure for reusability

When implementing new features, always check corresponding tests first to understand expected behavior and formats.

````

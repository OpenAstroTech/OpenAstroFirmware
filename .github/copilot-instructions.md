````instructions
# OpenAstroFirmware AI Agent Instructions

This is firmware for DIY astronomical telescope mounts, built on Zephyr RTOS v4.2.0. It's a modern C++20 rewrite of the successful OpenAstroTracker v1 firmware, emphasizing maintainability and advanced features.

## Architecture Overview

**Build System**: Zephyr RTOS v4.2.0 with west workspace management  
**Target Hardware**: MKS Robin Nano (STM32F407-based) boards + native simulation  
**Languages**: C++20 application layer, C protocol/driver layer  
**Module Structure**: Zephyr module with reusable components (`zephyr/module.yml`)

### Key Directories
- `app/` - Main Zephyr application with mount control logic
- `lib/lx200/` - LX200 telescope protocol implementation (C)
- `boards/mks/robin_nano/` - Custom board definition and device tree
- `drivers/` - Stepper motor and hardware drivers
- `tests/` - Comprehensive test suites using Zephyr Twister
- `include/` - Public headers (e.g., `include/lx200/lx200.h`)

## Development Workflow

### West Workspace Setup (Required)
This project uses west workspace management. Must be initialized properly:
```bash
# Initialize workspace (REQUIRED for first setup)
west init -m https://github.com/OpenAstroTech/OpenAstroFirmware --mr main .
west update  # Downloads Zephyr v4.2.0 + STM32 HAL modules
```

### Building & Testing
```bash
# From app/ directory
west build -b robin_nano                    # Production build
west build -b native_sim                    # Development/simulation
west build -b robin_nano -- -DEXTRA_CONF_FILE=debug.conf  # Debug build
west build -t pristine                      # Clean build

# Testing
west twister -T . -p robin_nano             # All tests on hardware
west twister -T tests/lib/lx200/             # Specific test suite
west twister -T . -p native_sim              # Simulation tests
```

### Configuration System
- **Kconfig**: `prj.conf` (base) + `debug.conf` (debug overlay)
- **Device Tree**: Custom STM32F407 board in `boards/mks/robin_nano/`
- **Board Configs**: `app/boards/native_sim.conf` for simulation
- **Interactive**: `west build -t menuconfig` / `west build -t guiconfig`

## Code Patterns & Conventions

### Language Separation Pattern
**C++20 Application Layer** (`app/src/`):
```cpp
#include <mount/Mount.hpp>
Mount mount;  // Main astronomical mount controller
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);  // Structured logging
```

**C Protocol/Driver Layer** (`lib/lx200/`, `drivers/`):
```c
#include <lx200/lx200.h>
lx200_parser_state_t parser_state;
LOG_MODULE_REGISTER(lx200, CONFIG_LX200_LOG_LEVEL);  // Module logging
```

### Zephyr Module Architecture
Project structure follows Zephyr module conventions:
- `zephyr/module.yml` - Module definition with board_root/dts_root
- `CMakeLists.txt` - Top-level module integration
- `Kconfig` - Module configuration options
- Device tree overlays in `dts/` and board-specific `boards/`

### Stepper Motor Driver Architecture
Multi-layer abstraction for telescope motor control:
```c
// Zephyr stepper driver API (include/zephyr/drivers/stepper.h)
stepper_move_by(dev, steps);
stepper_set_event_callback(dev, callback, user_data);

// TMC driver implementations (drivers/stepper/adi_tmc/)
// - TMC22xx: GPIO-based (step/dir pins)
// - TMC50xx/51xx: SPI-based with advanced features
```

### Device Tree Patterns
Hardware abstraction through device tree:
```dts
// boards/mks/robin_nano/robin_nano.dts
&spi1 {
    tmc_stepper: tmc5072@0 {
        compatible = "adi,tmc50xx";
        reg = <0>;
        // Stepper motor controller configuration
    };
};
```

## Testing Strategy

### Test-Driven Development Pattern
Tests serve as **specifications** for unimplemented features:

**Working Tests** (verify current functionality):
- Parser state management in `tests/lib/lx200/`
- Command family identification (A,B,C,D,F,G,g,H,I,L,M,P,Q,R,S,T,U)
- Basic command/parameter separation

**Specification Tests** (currently failing, define requirements):
- Coordinate parsing (RA: `HH:MM:SS`, Dec: `sDD*MM:SS`)
- Time/date parsing (`HH:MM:SS`, `MM/DD/YY`)
- Response formatting functions

### Multi-Configuration Testing
`app/sample.yaml` defines test configurations:
- `app.default`: Production configuration
- `app.debug`: Debug with `CONFIG_APP_LOG_LEVEL_DBG=y`

## LX200 Protocol Implementation

### Current Implementation Status
Commands follow `:COMMAND[params]#` format with **structured incompleteness**:
- ✅ **Parser Framework**: Command parsing, family classification
- ⏳ **Coordinate Functions**: All `lx200_parse_*_coordinate()` are stubs
- ⏳ **Formatting Functions**: All `lx200_format_*()` return errors

### Critical Implementation Pattern
All coordinate parsing stubs follow this pattern:
```c
lx200_parse_result_t lx200_parse_ra_coordinate(const char *str, lx200_coordinate_t *coord) {
    LOG_WRN("Function not implemented yet");
    return LX200_PARSE_ERROR;  // All coordinate functions are stubs
}
```

### Parameter Detection Issue
Current parameter detection is oversimplified:
```c
// NEEDS EXPANSION - only checks 'S' commands
bool lx200_command_has_parameter(const char *command) {
    return (command[0] == 'S');  // Missing many parameter commands
}
```

Missing parameter commands include: Rate commands (`R0`-`R9`), Special commands (`B+`, `B-`, `F+`, `F-`, `T+`, `T-`), Library commands, GPS commands.

## Astronomical Domain Context

### Coordinate Systems & Precision
- **RA/Dec**: Celestial coordinates for star tracking
- **Alt/Az**: Horizontal coordinates for mount positioning
- **Precision Modes**: High (`HH:MM:SS`) vs Low (`HH:MM.T`) formats
- **LX200 Compatibility**: Industry standard for ASCOM/INDI/Stellarium

### Hardware Integration
- **Stepper Motors**: RA/Dec axes with TMC drivers for precision
- **STM32F407**: 32-bit performance for real-time tracking calculations
- **Native Simulation**: PC-based development without hardware

## Development Guidelines

**When implementing new LX200 features:**
1. Check corresponding tests in `tests/lib/lx200/` first
2. Follow the established stub → implementation pattern
3. Use C for protocol layer, C++20 for application layer
4. Maintain Zephyr module structure for reusability

**When adding hardware support:**
1. Extend device tree in `boards/` or `dts/`
2. Follow Zephyr driver model conventions
3. Test with both hardware and native simulation
4. Update configuration in `Kconfig` files

This codebase is in **early development** - expect many stubs that define interfaces but await implementation. Always check tests to understand expected behavior before implementing.

````

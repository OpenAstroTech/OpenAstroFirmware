# OpenAstroFirmware AI Agent Instructions

This is firmware for DIY astronomical telescope mounts, built on Zephyr RTOS. It's a modern C++20 rewrite of the successful OpenAstroTracker v1 firmware, emphasizing maintainability and advanced features.

## Architecture Overview

**Build System**: Zephyr RTOS v4.1.0 with west build tool
**Target Hardware**: MKS Robin Nano (STM32F407-based) boards  
**Languages**: C++20 primary, with Zephyr C APIs
**Structure**: Modular Zephyr application with custom libraries and board support

Key directories:
- `firmware/` - Main application code with mount control logic
- `lib/lx200/` - LX200 telescope protocol implementation  
- `boards/mks/robin_nano/` - Custom board definition and device tree
- `zephyr/module.yml` - Zephyr module configuration

## Development Workflow

### Building
```bash
west build -b robin_nano                    # Standard build
west build -b robin_nano -- -DEXTRA_CONF_FILE=debug.conf  # Debug build
west twister -T . -p robin_nano             # Run tests via Twister
```

### Configuration
- **Kconfig**: Use `west build -t menuconfig` for configuration
- **Device Tree**: Hardware abstraction in `firmware/src/HardwareConfiguration.hpp`
- **Debug Mode**: Enable logging and debug optimizations via `debug.conf`

## Code Patterns & Conventions

### Logging
Every module uses structured logging:
```cpp
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ModuleName, CONFIG_APP_LOG_LEVEL);
// Then: LOG_INF(), LOG_DBG(), LOG_ERR()
```

### Hardware Abstraction
Use device tree patterns from `HardwareConfiguration.hpp`:
```cpp
namespace dt {
    static const struct device *uart_control_dev = DEVICE_DT_GET(DT_CHOSEN(oaf_uart_control));
}
```

### Mount Coordinates
Astronomical coordinate system throughout:
- **RA (Right Ascension)**: Hours:Minutes:Seconds (0-23:59:59)
- **Dec (Declination)**: Degrees:Minutes:Seconds (-90 to +90)
- Use precise astronomical formats for coordinate handling

### LX200 Protocol
Extensive Meade LX200 telescope protocol support in `lib/lx200/Lx200_Protocol.h`. Commands format: `:COMMAND#` with specific response patterns. Critical for telescope communication compatibility.

## Project-Specific Context

### Domain Knowledge
- **Telescope Mounts**: Alt-azimuth and equatorial mount types
- **Tracking**: Sidereal tracking for astronomical objects
- **Coordinates**: RA/Dec celestial coordinates, conversion between coordinate systems
- **Hardware**: Stepper motors, encoders, GPS modules for astronomical applications

### Integration Points
- **UART Communication**: Control commands via chosen UART device
- **Mount Hardware**: Stepper motor control and position feedback
- **LX200 Compatibility**: Industry-standard telescope communication protocol
- **USB Device Stack**: For PC connectivity and control

### Development Status
Early development phase - README states "work in progress, not for use/testing yet". Focus on architectural decisions and foundational implementations rather than end-user features.

## Testing Strategy
- **Twister Integration**: `sample.yaml` defines test configurations
- **Build-only Tests**: Primary testing via successful compilation for target hardware
- **Two Configurations**: Default (production-like) and debug (detailed logging)

When working on this codebase, prioritize understanding the astronomical coordinate systems, Zephyr's device model, and the mount control abstraction patterns that differentiate this from typical embedded projects.

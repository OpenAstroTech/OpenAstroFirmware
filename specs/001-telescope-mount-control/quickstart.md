# Developer Quickstart: Telescope Mount Control System

**Feature**: Telescope Mount Control System  
**Date**: 2025-10-12  
**Audience**: Developers implementing or testing this feature

## Overview

This guide helps you set up your development environment, understand the architecture, run tests, and contribute to the OpenAstroFirmware telescope mount control system.

## Prerequisites

### Required Software

1. **Python 3.8+** with virtual environment support
2. **West** (Zephyr's meta-tool)
3. **Zephyr SDK** (for ARM cross-compilation)
4. **CMake 3.20+**
5. **Ninja** build system
6. **Git**
7. **clang-format** (for code formatting)

### Hardware (Optional)

- **For testing**: None required (native_sim runs on your PC)
- **For deployment**: MKS Robin Nano or Nucleo F446RE board
- **For debugging**: SEGGER J-Link or ST-Link programmer

## Initial Setup

### 1. Install Zephyr Development Environment

Follow the [official Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) to install the Zephyr SDK.

Quick summary for Linux/macOS:
```bash
# Install west
pip3 install --user west

# Create Python virtual environment
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate

# Install Zephyr SDK
# Download from https://github.com/zephyrproject-rtos/sdk-ng/releases
# Extract and run setup script
```

### 2. Clone and Initialize Workspace

```bash
# Activate virtual environment
source ~/zephyrproject/.venv/bin/activate

# Create workspace directory
mkdir OpenAstroFirmware-workspace
cd OpenAstroFirmware-workspace

# Initialize west workspace
west init -m https://github.com/OpenAstroTech/OpenAstroFirmware --mr 001-telescope-mount-control .

# Update all Zephyr modules (downloads Zephyr RTOS v4.2.0 + HAL modules)
west update

# Install additional Python dependencies
pip install -r zephyr/scripts/requirements.txt
```

Your workspace structure:
```
OpenAstroFirmware-workspace/
├── OpenAstroFirmware/     # This repository (main application)
├── zephyr/                # Zephyr RTOS v4.2.0
└── modules/               # HAL modules (STM32, etc.)
```

### 3. Build Your First Test

```bash
cd OpenAstroFirmware-workspace/OpenAstroFirmware/app

# Build for native simulation (fast, no hardware needed)
west build -b native_sim

# Run on native_sim
west build -t run

# You should see:
# *** Booting Zephyr OS build v4.2.0 ***
# [00:00:00.000,000] <inf> main: OpenAstroFirmware v1.0.0
```

## Project Structure

```
OpenAstroFirmware/
├── app/                              # Main application (entry point)
│   ├── src/
│   │   ├── main.cpp                  # Application entry
│   │   ├── mount/                    # Mount control logic
│   │   └── protocol/                 # LX200 protocol handler
│   ├── boards/                       # Board-specific configs
│   │   ├── native_sim.conf
│   │   ├── robin_nano.conf
│   │   └── nucleo_f446re.conf
│   ├── prj.conf                      # Base configuration
│   └── CMakeLists.txt
│
├── lib/                              # Reusable libraries (C++20)
│   ├── lx200/                        # LX200 protocol (OOP rewrite)
│   │   ├── include/lx200/
│   │   │   ├── LX200Parser.hpp
│   │   │   ├── LX200Command.hpp
│   │   │   └── Commands/             # Command implementations
│   │   ├── src/
│   │   └── CMakeLists.txt
│   └── mount/                        # Mount control library
│
├── tests/                            # Test suites (Twister)
│   ├── lib/lx200/                    # LX200 library tests
│   └── app/                          # Application integration tests
│
└── specs/001-telescope-mount-control/  # Feature documentation
    ├── spec.md                       # Non-technical specification
    ├── plan.md                       # This implementation plan
    ├── research.md                   # Technology decisions
    ├── data-model.md                 # Data structures
    ├── contracts/                    # LX200 protocol contracts
    └── quickstart.md                 # This file
```

## Development Workflow

### 1. Test-Driven Development (TDD)

**Constitutional Requirement**: Tests MUST be written before implementation

```bash
# 1. Write a failing test
cd tests/lib/lx200
vim test_parser.cpp  # Add new test case

# 2. Run the test (should fail)
west twister -T tests/lib/lx200 -p native_sim
# Expected: ❌ Test fails

# 3. Implement minimal code to pass test
cd lib/lx200/src
vim LX200Parser.cpp  # Add implementation

# 4. Run test again (should pass)
west twister -T tests/lib/lx200 -p native_sim
# Expected: ✅ Test passes

# 5. Refactor if needed (tests still pass)

# 6. Commit
git add tests/lib/lx200/test_parser.cpp lib/lx200/src/LX200Parser.cpp
git commit -m "feat: add LX200 command parser with OOP design"
```

### 2. Running Tests

**Run all tests** (native_sim):
```bash
west twister -T tests -p native_sim
```

**Run specific test suite**:
```bash
# LX200 library tests only
west twister -T tests/lib/lx200 -p native_sim

# Mount control tests only
west twister -T tests/lib/mount -p native_sim

# Application integration tests
west twister -T tests/app -p native_sim
```

**Run with verbose output**:
```bash
west twister -T tests -p native_sim -v
```

**Run on hardware** (requires connected board):
```bash
# MKS Robin Nano
west twister -T tests -p robin_nano

# Nucleo F446RE
west twister -T tests -p nucleo_f446re
```

### 3. Building for Different Platforms

**Native Simulation** (development):
```bash
cd app
west build -b native_sim
west build -t run
```

**MKS Robin Nano** (production):
```bash
cd app
west build -b robin_nano
west flash  # Flash to connected board
```

**Nucleo F446RE** (testing):
```bash
cd app
west build -b nucleo_f446re
west flash
```

**Debug Build**:
```bash
cd app
west build -b robin_nano -- -DEXTRA_CONF_FILE=debug.conf
```

**Clean Build**:
```bash
west build -t pristine  # Remove all build artifacts
west build -b <board>   # Rebuild from scratch
```

### 4. Code Formatting

**Before every commit**, format your code:
```bash
# Format specific files
clang-format -i lib/lx200/src/LX200Parser.cpp
clang-format -i lib/lx200/include/lx200/LX200Parser.hpp

# Format all C++20 files in a directory
find lib/lx200 -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
```

**Check formatting** (CI will reject improperly formatted code):
```bash
clang-format --dry-run --Werror lib/lx200/src/*.cpp
```

### 5. Interactive Configuration

Modify Kconfig settings:
```bash
cd app
west build -t menuconfig  # Text-based UI
west build -t guiconfig   # GUI (requires dependencies)
```

Common configurations:
- `CONFIG_LOG_DEFAULT_LEVEL` - Logging verbosity (0-4)
- `CONFIG_MAIN_STACK_SIZE` - Main thread stack size
- `CONFIG_HEAP_MEM_POOL_SIZE` - Heap size for dynamic allocation

## Common Development Tasks

### Adding a New LX200 Command

1. **Define contract** (already done in `contracts/lx200-protocol.md`)
2. **Write test**:
   ```cpp
   // tests/lib/lx200/test_commands.cpp
   TEST(LX200Commands, GetRACommand) {
       GetRACommand cmd;
       MockMountController mount;
       mount.setRA(RightAscension{12, 34, 56});
       
       auto response = cmd.execute(mount);
       
       EXPECT_EQ(response.type(), LX200ResponseType::STRING);
       EXPECT_EQ(response.data(), "12:34:56#");
   }
   ```

3. **Implement command class**:
   ```cpp
   // lib/lx200/include/lx200/Commands/GetCommands.hpp
   class GetRACommand : public LX200Command {
   public:
       LX200Response execute(MountController& mount) override {
           auto ra = mount.currentPosition().celestialPosition().ra();
           return LX200Response::string(ra.toString() + "#");
       }
       
       bool requiresParameter() const noexcept override { return false; }
       std::string_view commandName() const noexcept override { return "GR"; }
   };
   ```

4. **Register in parser**:
   ```cpp
   // lib/lx200/src/LX200Parser.cpp
   LX200Parser::LX200Parser() {
       registerCommand("GR", std::make_unique<GetRACommandFactory>());
       // ... other commands
   }
   ```

5. **Test**:
   ```bash
   west twister -T tests/lib/lx200 -p native_sim
   ```

### Debugging on Hardware

**Using GDB with J-Link**:
```bash
cd app
west build -b robin_nano -- -DEXTRA_CONF_FILE=debug.conf
west debug

# In GDB:
(gdb) break main
(gdb) continue
(gdb) print mount_position
```

**Using Serial Monitor**:
```bash
# Connect to UART (115200 baud)
minicom -D /dev/ttyUSB0 -b 115200

# Or using screen
screen /dev/ttyUSB0 115200
```

**Viewing Logs**:
```bash
# Logs go to UART by default
# Increase verbosity in debug.conf:
# CONFIG_LOG_DEFAULT_LEVEL=4  # DEBUG level
```

### Adding Board-Specific Configuration

1. **Create device tree overlay**:
   ```dts
   // app/boards/my_custom_board.overlay
   / {
       mount_config {
           compatible = "openastro,mount-config";
           ra-steps-per-revolution = <4608000>;
           dec-steps-per-revolution = <4608000>;
           // ... other config
       };
   };
   ```

2. **Create Kconfig file**:
   ```ini
   # app/boards/my_custom_board.conf
   CONFIG_SERIAL=y
   CONFIG_UART_INTERRUPT_DRIVEN=y
   # ... board-specific settings
   ```

3. **Build for new board**:
   ```bash
   west build -b my_custom_board
   ```

## Testing Strategy

### Unit Tests (Fast, No Hardware)

Run on native_sim for rapid iteration:
```bash
# All unit tests
west twister -T tests/lib -p native_sim

# Specific test file
west twister -T tests/lib/lx200/test_parser.cpp -p native_sim
```

**What to test**:
- LX200 command parsing
- Coordinate conversions
- Tracking calculations
- Calibration math
- State machine transitions

### Integration Tests (Medium, Simulated Hardware)

Test component interactions:
```bash
west twister -T tests/app -p native_sim
```

**What to test**:
- LX200 handler → Mount controller interaction
- Command sequences (Sr, Sd, MS, D)
- Tracking loop integration
- Error handling flows

### Hardware Tests (Slow, Real Hardware)

Final validation on target hardware:
```bash
# Flash test firmware
west twister -T tests -p robin_nano --device-testing --device-serial /dev/ttyUSB0
```

**What to test**:
- Actual stepper motor movement
- Real-time timing validation
- Device tree configuration
- Interrupt latency

## Performance Profiling

### Memory Usage

```bash
# Build with size optimization
west build -b robin_nano -- -DCONFIG_SIZE_OPTIMIZATIONS=y

# Check binary size
arm-none-eabi-size build/zephyr/zephyr.elf

# Expected output:
#    text    data     bss     dec     hex filename
#   85432    2048   18512  105992   19df8 build/zephyr/zephyr.elf
#
# RAM usage = data + bss = ~20KB (well within 128KB limit)
```

### Latency Measurements

Add timing instrumentation:
```cpp
#include <zephyr/timing/timing.h>

timing_t start = timing_counter_get();
// ... code to measure
timing_t end = timing_counter_get();
uint64_t cycles = timing_cycles_get(&start, &end);
uint64_t ns = timing_cycles_to_ns(cycles);

LOG_INF("Command latency: %llu ns", ns);
```

## Troubleshooting

### Build Errors

**"West command not found"**:
```bash
# Activate virtual environment
source ~/zephyrproject/.venv/bin/activate
```

**"No rule to make target zephyr"**:
```bash
# Your west workspace might not be initialized
cd OpenAstroFirmware-workspace
west update
```

**"undefined reference to ..."**:
- Check CMakeLists.txt for missing `target_link_libraries`
- Verify library is added to build system

### Test Failures

**Tests timeout on native_sim**:
- Check for infinite loops
- Verify `k_sleep()` calls use reasonable timeouts
- Enable debug logging: `CONFIG_LOG_DEFAULT_LEVEL=4`

**Tests pass on native_sim but fail on hardware**:
- Check device tree configuration
- Verify hardware-specific timings
- Add hardware-specific test cases

### Runtime Issues

**Mount doesn't respond to commands**:
```bash
# Check serial connection
ls -l /dev/ttyUSB*  # or /dev/ttyACM*

# Verify baud rate (should be 115200)
stty -F /dev/ttyUSB0

# Test with minicom
minicom -D /dev/ttyUSB0 -b 115200
# Type: :GR#
# Expected response: HH:MM:SS#
```

**Tracking accuracy issues**:
- Verify calibration (2-3 alignment stars)
- Check sidereal rate: 15.041067 arcsec/sec
- Review device tree motor configuration
- Measure actual motor step rate

## Contributing

### Before Creating PR

1. ✅ All tests pass on native_sim
2. ✅ Code formatted with clang-format
3. ✅ Constitution compliance verified
4. ✅ Tests pass on hardware (if hardware changes)
5. ✅ Documentation updated (if API changes)
6. ✅ Commit messages follow conventional commits

### PR Checklist

- [ ] Feature branch named `###-feature-name`
- [ ] Tests written before implementation (TDD)
- [ ] All tests pass (`west twister -T tests -p native_sim`)
- [ ] Code formatted (`clang-format -i`)
- [ ] Memory usage within budget (<128KB RAM)
- [ ] Command latency <100ms measured
- [ ] No unnecessary complexity added
- [ ] Documentation updated

### Code Review Focus

Reviewers will check:
1. **Test coverage**: Are all paths tested?
2. **Simplicity**: Can this be simpler?
3. **Constitution compliance**: TDD, C++20, minimal complexity?
4. **Performance**: Meets latency/memory requirements?
5. **Error handling**: Proper logging, return codes?

## Next Steps

1. **Read the specification**: `specs/001-telescope-mount-control/spec.md`
2. **Understand the architecture**: `specs/001-telescope-mount-control/research.md`
3. **Review data models**: `specs/001-telescope-mount-control/data-model.md`
4. **Study protocol contracts**: `specs/001-telescope-mount-control/contracts/lx200-protocol.md`
5. **Start with tests**: Pick a user story, write failing tests, implement

## Resources

- **Zephyr Documentation**: https://docs.zephyrproject.org/4.2.0/
- **LX200 Protocol Reference**: `lx200-protocol-llm-reference.md`
- **Project Constitution**: `.specify/memory/constitution.md`
- **Discord Community**: https://discord.gg/pzGaJztdAD

## Getting Help

- **Test failures**: Check logs with `CONFIG_LOG_DEFAULT_LEVEL=4`
- **Build issues**: Verify west workspace with `west update`
- **Hardware problems**: Test on native_sim first
- **Architecture questions**: Review `research.md` and `data-model.md`
- **Protocol issues**: Consult `contracts/lx200-protocol.md`

Welcome to OpenAstroFirmware development! 🔭✨

# Implementation Plan: Telescope Mount Control System

**Branch**: `001-telescope-mount-control` | **Date**: 2025-10-12 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/001-telescope-mount-control/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Implement a complete telescope mount control system for DIY astronomical mounts using Zephyr RTOS v4.2.0. The system enables amateur astronomers and astrophotographers to control telescope mounts from astronomy software via the industry-standard LX200 protocol, with precise celestial tracking for long-exposure imaging. The implementation will completely rewrite the LX200 library using C++20 OOP principles while maintaining simplicity, modularity, testability, and maintainability. All tests must run on native_sim or QEMU to minimize hardware dependencies during development.

## Technical Context

**Language/Version**: C++20 (application/libraries), C (low-level drivers only)  
**Primary Dependencies**: 
- Zephyr RTOS v4.2.0
- West build system
- CMake 3.20+
- Zephyr SDK (ARM Cortex-M4 toolchain)
- clang-format (code formatting)
- Twister (test framework)

**Storage**: 
- Non-volatile: Device tree configuration files (.dts/.overlay)
- Volatile: RAM-based state (mount position, calibration, tracking state)
- Persistent calibration data via Zephyr settings subsystem

**Testing**: 
- Zephyr Twister test framework
- Unit tests on native_sim platform
- Integration tests on native_sim/QEMU
- Hardware validation on robin_nano (STM32F407) and nucleo_f446re

**Target Platform**: 
- Primary: MKS Robin Nano (STM32F407VET6, 512KB Flash, 192KB RAM)
- Secondary: Nucleo F446RE (STM32F446RET6, 512KB Flash, 128KB RAM)
- Development: native_sim (x86_64 Linux simulation)
- Future: Additional 32-bit ARM Cortex-M boards

**Project Type**: Embedded firmware (single unified codebase)

**Performance Goals**:
- LX200 command response latency: <100ms (synchronous commands)
- Tracking precision: ±5 arcseconds maintained continuously
- Stepper control loop frequency: 1kHz minimum for smooth motion
- Memory footprint: <128KB RAM on STM32F407 (leaving headroom)
- CPU utilization: <50% average to allow for background tasks

**Constraints**:
- Real-time determinism required for tracking accuracy
- Memory-constrained embedded environment (128-192KB RAM)
- No dynamic memory allocation in critical paths
- LX200 protocol cannot communicate errors (logging only)
- Must work on native_sim for development/testing without hardware
- Single codebase must support multiple boards via device tree

**Scale/Scope**:
- ~10K lines of C++20 application/library code
- ~2K lines of C driver code
- ~50 LX200 protocol commands to implement
- 3 target platforms (native_sim, robin_nano, nucleo_f446re)
- 6 user stories with 20+ acceptance scenarios
- Comprehensive test suite runnable without hardware

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Principle I: Test-First Development ✅

- **Status**: PASS
- **Compliance**: All features will follow TDD cycle with tests written before implementation
- **Evidence**: Plan includes comprehensive test strategy using Zephyr Twister on native_sim
- **Action**: Tests MUST be written and failing before any implementation begins

### Principle II: Code Quality & Maintainability ✅

- **Language Separation**: ✅ C++20 for app/lib, C for drivers
- **Minimal Complexity**: ✅ Explicitly requested "as simple as possible, don't overengineer"
- **Consistent Formatting**: ✅ .clang-format already exists
- **Zephyr Module Structure**: ✅ Following existing zephyr/module.yml conventions
- **Structured Logging**: ✅ Will use LOG_MODULE_REGISTER throughout
- **Device Tree First**: ✅ Board-specific configurations via device tree overlays
- **No Magic Numbers**: ✅ Astronomical constants will be named and documented
- **Modern C++ Features**: ✅ C++20 features used only when they simplify code

### Principle III: Real-Time Performance & Reliability ✅

- **Tracking Precision**: ✅ Target: ±5 arcseconds (per spec SC-005)
- **Command Latency**: ✅ Target: <100ms (aligned with constitution)
- **Memory Constraints**: ✅ Must fit in 128KB RAM (STM32F407 has 192KB)
- **Deterministic Timing**: ✅ Will use Zephyr RTOS priority threads
- **Fail-Safe Mechanisms**: ✅ Timeout-based safety stops planned
- **No Blocking Operations**: ✅ Async I/O for serial communication

### Principle IV: User Experience Consistency ✅

- **LX200 Protocol Compliance**: ✅ Complete rewrite to ensure compatibility
- **Cross-Platform Parity**: ✅ native_sim, robin_nano, nucleo_f446re support
- **Coordinate Format Standards**: ✅ Strict HH:MM:SS / sDD*MM:SS adherence
- **Error Communication**: ✅ Logging-based error reporting (LX200 limitation)
- **Configuration Portability**: ✅ Device tree overlays for board-specific config
- **Simulation Fidelity**: ✅ Tests run on native_sim before hardware

### Quality Gates & Standards ✅

- **West Workspace**: ✅ Already using west build system
- **Configuration Overlays**: ✅ prj.conf and debug.conf maintained
- **Multi-Board Support**: ✅ Three platforms supported
- **Clean Builds**: ✅ Will enforce `west build -t pristine` before PR merge
- **Documentation Standards**: ✅ ADRs, requirements traceability, inline docs
- **Development Workflow**: ✅ Feature branch, spec-first, incremental delivery, CI/CD

### Gate Evaluation

**RESULT**: ✅ **PASS** - No violations requiring justification

All constitutional principles are satisfied by the planned approach:
- Test-first with native_sim eliminates hardware dependency barrier
- C++20 simplicity focus aligns with minimal complexity principle
- Real-time performance targets match constitutional requirements
- LX200 protocol rewrite ensures user experience consistency
- Zephyr RTOS infrastructure provides required quality gates

## Project Structure

### Documentation (this feature)

```
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```
OpenAstroFirmware/                    # Repository root
├── app/                              # Main Zephyr application (C++20)
│   ├── src/
│   │   ├── main.cpp                  # Application entry point
│   │   ├── mount/
│   │   │   ├── Mount.hpp/cpp         # Main mount controller
│   │   │   ├── Coordinate.hpp/cpp    # Celestial coordinate handling
│   │   │   ├── Tracking.hpp/cpp      # Tracking state machine
│   │   │   └── Calibration.hpp/cpp   # Alignment & calibration
│   │   └── protocol/
│   │       └── LX200Handler.hpp/cpp  # LX200 protocol handler
│   ├── boards/                       # Board-specific configs
│   │   ├── native_sim.conf
│   │   ├── native_sim.overlay
│   │   ├── robin_nano.conf
│   │   ├── robin_nano.overlay
│   │   ├── nucleo_f446re.conf
│   │   └── nucleo_f446re.overlay
│   ├── CMakeLists.txt
│   ├── prj.conf                      # Base configuration
│   ├── debug.conf                    # Debug overlay
│   └── sample.yaml                   # Twister test configurations
│
├── lib/                              # Reusable libraries (C++20)
│   ├── lx200/                        # LX200 protocol library (REWRITE)
│   │   ├── include/lx200/
│   │   │   ├── LX200Parser.hpp       # Command parser (OOP)
│   │   │   ├── LX200Command.hpp      # Command base class
│   │   │   ├── LX200Response.hpp     # Response formatter
│   │   │   └── Commands/             # Specific command classes
│   │   │       ├── SyncCommands.hpp
│   │   │       ├── MotionCommands.hpp
│   │   │       ├── GetCommands.hpp
│   │   │       └── SetCommands.hpp
│   │   ├── src/
│   │   │   ├── LX200Parser.cpp
│   │   │   ├── LX200Response.cpp
│   │   │   └── Commands/
│   │   ├── CMakeLists.txt
│   │   └── Kconfig
│   └── mount/                        # Mount control library
│       ├── include/mount/
│       │   ├── StepperController.hpp
│       │   ├── MotionPlanner.hpp
│       │   └── CoordinateSystem.hpp
│       └── src/
│
├── drivers/                          # Low-level drivers (C only)
│   ├── stepper/
│   │   └── adi_tmc/                  # TMC stepper drivers (existing)
│   └── CMakeLists.txt
│
├── tests/                            # Test suites
│   ├── lib/
│   │   ├── lx200/                    # LX200 library tests
│   │   │   ├── test_parser.cpp
│   │   │   ├── test_commands.cpp
│   │   │   ├── test_coordinates.cpp
│   │   │   └── testcase.yaml
│   │   └── mount/                    # Mount library tests
│   │       ├── test_tracking.cpp
│   │       ├── test_calibration.cpp
│   │       └── testcase.yaml
│   └── app/                          # Application integration tests
│       ├── test_mount_control.cpp
│       ├── test_lx200_integration.cpp
│       └── testcase.yaml
│
├── include/                          # Public headers
│   ├── lx200/                        # LX200 library public API
│   └── mount/                        # Mount library public API
│
├── boards/                           # Custom board definitions
│   └── mks/
│       └── robin_nano/               # MKS Robin Nano board support
│
├── dts/                              # Device tree bindings
│   └── bindings/
│
├── specs/                            # Feature specifications
│   └── 001-telescope-mount-control/
│       ├── spec.md                   # This feature's spec
│       ├── plan.md                   # This file
│       ├── research.md               # Phase 0 output
│       ├── data-model.md             # Phase 1 output
│       ├── quickstart.md             # Phase 1 output
│       ├── contracts/                # Phase 1 output
│       └── checklists/
│
├── zephyr/                           # Zephyr module definition
│   └── module.yml
│
├── CMakeLists.txt                    # Root CMake
├── Kconfig                           # Root Kconfig
├── west.yml                          # West manifest
└── .clang-format                     # Code formatting rules
```

**Structure Decision**: Embedded firmware structure using Zephyr RTOS conventions

This is a **single unified embedded firmware project** (not web/mobile). The structure follows Zephyr module conventions with:

1. **app/**: Main application code (C++20) - mount control logic, LX200 handler
2. **lib/**: Reusable libraries (C++20) - LX200 protocol (complete rewrite), mount control
3. **drivers/**: Hardware drivers (C only) - stepper motor control, low-level HAL
4. **tests/**: Comprehensive test suites runnable on native_sim
5. **boards/**: Custom board definitions following Zephyr conventions
6. **include/**: Public library headers for external consumption

Key architectural decisions:
- **LX200 library complete OOP rewrite** in C++20 under `lib/lx200/`
- **Separation**: Application in `app/`, libraries in `lib/`, drivers in `drivers/`
- **Testing**: All tests under `tests/` with Twister integration
- **Device tree**: Board-specific overlays in `app/boards/` and custom boards in `boards/`

## Complexity Tracking

*Fill ONLY if Constitution Check has violations that must be justified*

**No violations** - All constitutional principles satisfied by planned approach.

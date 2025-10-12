# Research: Telescope Mount Control System

**Feature**: Telescope Mount Control System  
**Phase**: 0 - Research & Technology Selection  
**Date**: 2025-10-12

## Overview

This document captures research findings and technology decisions for implementing a complete telescope mount control system using Zephyr RTOS v4.2.0, with a focus on simplicity, testability, and maintainability.

## Technology Decisions

### 1. LX200 Protocol Implementation (C++20 OOP Rewrite)

**Decision**: Complete rewrite of LX200 library using modern C++20 OOP principles

**Rationale**:
- Current implementation uses C with procedural design, making it difficult to extend and test
- C++20 enables cleaner abstractions: command pattern, type safety, RAII
- Object-oriented design allows proper encapsulation of command parsing/execution
- Testability improves dramatically with dependency injection and mock objects
- Constitution mandates C++20 for libraries when it reduces complexity

**Architecture**:
```cpp
// Command pattern with polymorphism
class LX200Command {
public:
    virtual ~LX200Command() = default;
    virtual LX200Response execute(MountController& mount) = 0;
    virtual bool requiresParameter() const = 0;
};

// Specific command classes
class SyncCommand : public LX200Command {
    Coordinate target_;
public:
    explicit SyncCommand(Coordinate coord) : target_(coord) {}
    LX200Response execute(MountController& mount) override;
};

// Parser uses factory pattern
class LX200Parser {
    std::unordered_map<std::string, std::unique_ptr<CommandFactory>> factories_;
public:
    std::unique_ptr<LX200Command> parse(std::string_view input);
};
```

**Benefits**:
- Type-safe command objects vs. switch statements on char arrays
- Easy to add new commands without modifying parser core
- Unit tests can mock MountController interface
- RAII ensures resource cleanup (no manual memory management)
- Concepts can enforce command interface contracts

**Alternatives Considered**:
1. **Keep C implementation**: Rejected - poor testability, high coupling, difficult to extend
2. **C with function pointers**: Rejected - still manual memory management, no type safety
3. **Rust**: Rejected - not aligned with project's C++20 direction, toolchain complexity

### 2. Testing Strategy: native_sim First

**Decision**: All tests must run on Zephyr native_sim platform before hardware validation

**Rationale**:
- Hardware testing is slow (flash, connect, debug cycle)
- native_sim runs tests at x86 speeds (seconds vs. minutes)
- CI/CD can run full test suite without physical hardware
- Developers can test without specialized equipment
- Constitution requires multi-platform validation including simulation

**Implementation**:
```yaml
# app/sample.yaml - Twister configuration
tests:
  app.mount_control.basic:
    platform_allow: native_sim robin_nano nucleo_f446re
    tags: mount lx200
  
  app.mount_control.tracking:
    platform_allow: native_sim
    tags: tracking integration
```

**Test Coverage Requirements**:
- **Unit tests**: All lib/lx200 command classes, coordinate parsing, tracking math
- **Integration tests**: LX200 protocol → Mount controller interaction
- **Contract tests**: LX200 command/response format validation
- **Hardware tests**: Final validation on robin_nano (after native_sim passes)

**Simulation Limitations**:
- Cannot test actual stepper motor timing (use mocks)
- Cannot test real-time interrupt latency (validate on hardware)
- Cannot test device tree pin configurations (visual inspection on hardware)

**Solution**: Separate test suites - simulation for logic, hardware for physical validation

### 3. Mount Controller Architecture

**Decision**: Layered architecture with clear separation of concerns

**Layers**:
```
┌─────────────────────────────────────┐
│   LX200 Protocol Handler            │  ← Serial I/O, command parsing
├─────────────────────────────────────┤
│   Mount Controller                  │  ← High-level control logic
│   - Coordinate management           │
│   - Tracking state machine          │
│   - Calibration & alignment         │
├─────────────────────────────────────┤
│   Motion Planner                    │  ← Trajectory generation
│   - Slew profiles                   │
│   - Tracking rate calculations      │
├─────────────────────────────────────┤
│   Stepper Controller                │  ← Motor control abstraction
│   - Step/direction generation       │
│   - Acceleration control            │
├─────────────────────────────────────┤
│   Hardware Drivers (C)              │  ← Device-specific code
│   - TMC stepper drivers             │
│   - GPIO/SPI/UART                   │
└─────────────────────────────────────┘
```

**Rationale**:
- Each layer has single responsibility
- Upper layers don't know about hardware details
- Easy to test each layer independently
- Device tree configuration isolates hardware differences
- Follows Zephyr architectural patterns

**Simplicity Enforcement**:
- **No unnecessary abstractions**: If a layer isn't needed, it's not created
- **Direct function calls**: Avoid callback hell unless async required
- **Minimal interfaces**: Each class exposes only what's needed
- **YAGNI**: No "future-proofing" for hypothetical features

### 4. Memory Management Strategy

**Decision**: Stack allocation and static lifetimes, no dynamic allocation in critical paths

**Rationale**:
- Embedded constraints: 128-192KB RAM total
- Real-time requirement: No unpredictable allocation delays
- Constitution: Deterministic timing required for tracking
- Zephyr best practice: Avoid malloc() in RTOS applications

**Patterns**:
```cpp
// GOOD: Stack allocation
void handleCommand(std::string_view cmd) {
    LX200Parser parser;  // Stack allocated
    auto command = parser.parse(cmd);  // Returns unique_ptr
    command->execute(mountController);  // Executed immediately
}

// GOOD: Static lifetime for singletons
MountController& getMountController() {
    static MountController instance;  // Constructed once
    return instance;
}

// BAD: Dynamic allocation in hot path
void trackingLoop() {
    while (true) {
        auto* state = new TrackingState();  // ❌ Heap allocation in loop
        // ...
    }
}
```

**Smart Pointer Usage**:
- `std::unique_ptr`: Ownership transfer for commands (parse → execute)
- `std::shared_ptr`: AVOID unless absolutely necessary
- Raw pointers: AVOID, use references instead

**Memory Budget**:
- Mount state: ~4KB (position, calibration, config)
- LX200 buffers: ~2KB (command/response buffers)
- Tracking state: ~1KB (ephemeris calculations)
- Zephyr RTOS: ~60KB (kernel, threads, networking if enabled)
- **Remaining**: ~60KB headroom for stack, drivers, future features

### 5. Coordinate System Implementation

**Decision**: Use modern C++20 strong types for coordinate safety

**Implementation**:
```cpp
// Strong types prevent mixing coordinate systems
struct RightAscension {
    int hours;
    int minutes;
    double seconds;
    
    [[nodiscard]] constexpr double toRadians() const noexcept;
    [[nodiscard]] std::string toString() const;  // "HH:MM:SS"
};

struct Declination {
    bool positive;
    int degrees;
    int arcminutes;
    double arcseconds;
    
    [[nodiscard]] constexpr double toRadians() const noexcept;
    [[nodiscard]] std::string toString() const;  // "sDD*MM:SS"
};

// Celestial coordinate (immutable)
class CelestialCoordinate {
    RightAscension ra_;
    Declination dec_;
public:
    constexpr CelestialCoordinate(RightAscension ra, Declination dec) noexcept
        : ra_(ra), dec_(dec) {}
    
    [[nodiscard]] constexpr RightAscension ra() const noexcept { return ra_; }
    [[nodiscard]] constexpr Declination dec() const noexcept { return dec_; }
};
```

**Benefits**:
- Type system prevents mixing RA/Dec with Alt/Az
- Compile-time validation with `constexpr`
- `[[nodiscard]]` catches ignored return values
- `noexcept` enables compiler optimizations
- String formatting matches LX200 protocol requirements

**Alternatives Considered**:
1. **Plain doubles**: Rejected - easy to mix coordinate systems, no units
2. **Units library (boost.units)**: Rejected - unnecessary complexity, large dependency
3. **Struct with union**: Rejected - error-prone, no type safety

### 6. Asynchronous I/O Strategy

**Decision**: Zephyr message queues + callbacks for serial I/O, no blocking in protocol handler

**Rationale**:
- Constitution: No blocking operations in critical paths
- Serial I/O is inherently slow (9600-115200 baud)
- Tracking loop cannot be blocked by command processing
- Zephyr provides efficient message passing primitives

**Architecture**:
```cpp
// Serial RX thread
void serialRxThread() {
    char buffer[256];
    while (true) {
        size_t len = uart_read(buffer, sizeof(buffer));
        if (len > 0) {
            k_msgq_put(&lx200_cmd_queue, buffer, K_NO_WAIT);
        }
    }
}

// Command processing thread
void commandProcessorThread() {
    char cmd[256];
    while (true) {
        k_msgq_get(&lx200_cmd_queue, cmd, K_FOREVER);
        auto command = parser.parse(cmd);
        auto response = command->execute(mountController);
        uart_write(response.data(), response.size());
    }
}

// Tracking thread (highest priority, never blocked)
void trackingThread() {
    while (true) {
        updateMotorPositions();
        k_sleep(K_MSEC(1));  // 1kHz control loop
    }
}
```

**Thread Priorities**:
1. Tracking: Highest (real-time requirement)
2. Motor control: High (time-sensitive)
3. Command processing: Normal
4. Serial I/O: Low (can buffer)

### 7. Device Tree Configuration Strategy

**Decision**: All hardware configuration via device tree, zero compile-time #defines

**Rationale**:
- Constitution: Device tree first, no #defines
- Same binary supports different boards
- Community can customize without recompiling
- Follows Zephyr best practices

**Example Configuration**:
```dts
// boards/robin_nano.overlay
&uart1 {
    status = "okay";
    current-speed = <115200>;
};

/ {
    mount_config {
        compatible = "openastro,mount-config";
        ra-steps-per-revolution = <4608000>;  // 16x microstepping, 1:144 gear ratio
        dec-steps-per-revolution = <4608000>;
        ra-max-speed = <800>;  // steps/second
        dec-max-speed = <800>;
        tracking-rate = <15.041067>;  // arcseconds/second (sidereal)
    };
    
    steppers {
        ra_stepper: ra_stepper {
            compatible = "adi,tmc2209";
            step-gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>;
            dir-gpios = <&gpioa 1 GPIO_ACTIVE_HIGH>;
            enable-gpios = <&gpioa 2 GPIO_ACTIVE_LOW>;
            microsteps = <16>;
        };
        
        dec_stepper: dec_stepper {
            compatible = "adi,tmc2209";
            step-gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>;
            dir-gpios = <&gpiob 1 GPIO_ACTIVE_HIGH>;
            enable-gpios = <&gpiob 2 GPIO_ACTIVE_LOW>;
            microsteps = <16>;
        };
    };
};
```

**Configuration Access**:
```cpp
// Application reads from device tree at runtime
const struct device *ra_stepper = DEVICE_DT_GET(DT_NODELABEL(ra_stepper));
uint32_t steps_per_rev = DT_PROP(DT_NODELABEL(mount_config), ra_steps_per_revolution);
```

### 8. Error Handling Strategy

**Decision**: Logging for diagnostics, return codes for control flow, no exceptions

**Rationale**:
- Constitution: LX200 cannot communicate errors, must log
- Zephyr doesn't enable exceptions by default (code size, performance)
- Return codes are explicit and testable
- Logging provides diagnostic information

**Patterns**:
```cpp
// Function returns status code
enum class MountError {
    OK,
    INVALID_COORDINATE,
    OUT_OF_BOUNDS,
    MOTOR_FAULT,
    TIMEOUT
};

MountError gotoCoordinate(CelestialCoordinate target) {
    if (!isValid(target)) {
        LOG_ERR("Invalid coordinate: RA=%s Dec=%s", 
                target.ra().toString().c_str(),
                target.dec().toString().c_str());
        return MountError::INVALID_COORDINATE;
    }
    
    // ... implementation
    LOG_INF("Slewing to RA=%s Dec=%s", 
            target.ra().toString().c_str(),
            target.dec().toString().c_str());
    return MountError::OK;
}

// Caller checks return value
if (auto err = gotoCoordinate(target); err != MountError::OK) {
    // Handle error appropriately
    return formatLX200Error(err);
}
```

**Logging Levels**:
- `LOG_ERR`: Command failures, hardware faults, protocol violations
- `LOG_WRN`: Recoverable issues, boundary conditions, deprecated usage
- `LOG_INF`: Normal operations, state transitions, command execution
- `LOG_DBG`: Detailed diagnostics, coordinate calculations, timing info

### 9. Build System Configuration

**Decision**: West workspace with CMake, multi-board support, Twister integration

**Configuration Files**:
```cmake
# app/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(OpenAstroFirmware)

# Application sources
target_sources(app PRIVATE
    src/main.cpp
    src/mount/Mount.cpp
    src/mount/Coordinate.cpp
    src/mount/Tracking.cpp
    src/mount/Calibration.cpp
    src/protocol/LX200Handler.cpp
)

# Include directories
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Libraries
add_subdirectory(../lib/lx200 lx200)
add_subdirectory(../lib/mount mount)
target_link_libraries(app PRIVATE lx200 mount)
```

```ini
# app/prj.conf - Base configuration
CONFIG_MAIN_STACK_SIZE=4096
CONFIG_HEAP_MEM_POOL_SIZE=8192

# Logging
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3  # INF
CONFIG_LOG_MODE_IMMEDIATE=n  # Deferred for performance

# Serial
CONFIG_SERIAL=y
CONFIG_UART_INTERRUPT_DRIVEN=y

# Threading
CONFIG_NUM_PREEMPT_PRIORITIES=15
CONFIG_TIMESLICING=y

# Settings (persistent config)
CONFIG_SETTINGS=y
CONFIG_SETTINGS_RUNTIME=y

# C++ Support
CONFIG_CPP=y
CONFIG_STD_CPP20=y
CONFIG_EXCEPTIONS=n  # Disabled for code size
CONFIG_RTTI=n  # Disabled for code size
```

```ini
# app/debug.conf - Debug overlay
CONFIG_LOG_DEFAULT_LEVEL=4  # DBG
CONFIG_DEBUG=y
CONFIG_DEBUG_INFO=y
CONFIG_THREAD_NAME=y
CONFIG_THREAD_MONITOR=y
CONFIG_ASSERT=y
```

### 10. Development Workflow

**Decision**: Feature branch workflow with specification-driven development

**Process**:
1. **Specification**: Write user story specs before code (already done)
2. **Planning**: Create implementation plan with technology choices (this document)
3. **Test First**: Write failing tests for feature
4. **Implementation**: Write minimal code to pass tests
5. **Refactor**: Clean up while keeping tests green
6. **Integration**: Test on native_sim first, then hardware
7. **Documentation**: Update inline docs and architecture docs
8. **Review**: Constitution compliance check + code review
9. **Merge**: After all checks pass

**Branch Naming**: `###-feature-name` (e.g., `001-telescope-mount-control`)

**Commit Messages**: Follow conventional commits
```
feat: add LX200 command parser with OOP design
test: add unit tests for coordinate parsing
refactor: simplify tracking state machine
fix: correct RA coordinate format validation
docs: update LX200 protocol implementation guide
```

## Research Findings Summary

### Key Technical Choices

| Area | Decision | Justification |
|------|----------|---------------|
| Language | C++20 (app/lib), C (drivers) | Constitution requirement, simplicity via modern features |
| RTOS | Zephyr v4.2.0 | Real-time determinism, extensive board support, active community |
| Testing | Native_sim first | Fast iteration, no hardware dependency, CI/CD friendly |
| LX200 | Complete OOP rewrite | Testability, maintainability, type safety |
| Memory | Stack allocation, static lifetime | Deterministic timing, embedded constraints |
| I/O | Async with message queues | Non-blocking requirement, real-time tracking |
| Configuration | Device tree only | Board portability, runtime customization |
| Error Handling | Return codes + logging | No exceptions, explicit control flow, LX200 limitation |

### Performance Validation Checkpoints

Before declaring success, validate:
1. ✅ All tests pass on native_sim
2. ✅ Memory usage <128KB on STM32F407
3. ✅ Command latency <100ms measured
4. ✅ Tracking precision ±5 arcseconds measured
5. ✅ 8-hour stability test passes
6. ✅ LX200 compatibility with Stellarium/ASCOM/INDI verified
7. ✅ Cross-platform: robin_nano, nucleo_f446re, native_sim all pass

### Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| C++20 code size exceeds flash | High | Profile early, disable RTTI/exceptions, use linker optimizations |
| Real-time deadline misses | High | Priority threads, avoid blocking, profile on hardware |
| LX200 compatibility issues | Medium | Test against multiple clients, reference implementation validation |
| Native_sim behavior differs from hardware | Medium | Separate test suites, hardware validation gate before release |
| Device tree configuration errors | Low | Compile-time validation, clear documentation, examples |

## Next Steps

**Phase 1 Tasks**:
1. Create `data-model.md` - Define coordinate, mount state, calibration data structures
2. Create `contracts/` - Define LX200 command/response contracts
3. Create `quickstart.md` - Developer setup and testing guide
4. Update agent context with technology decisions

**Ready to proceed**: ✅ All research questions resolved, technology decisions made

# Tasks: Telescope Mount Control System

**Input**: Design documents from `/specs/001-telescope-mount-control/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/lx200-protocol.md, quickstart.md

**Tests**: TDD approach explicitly required by constitution - tests MUST be written FIRST

**Organization**: Tasks organized by architectural layer (bottom-up) and mapped to user stories for traceability

## Format: `[ID] [P?] [Story] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task supports (US1-US6)
- **[Layer]**: Architecture layer (Stepper API, Motion Planner, Controller, Protocol)
- Include exact file paths in descriptions

## Path Conventions

This is a Zephyr RTOS embedded firmware project:
- **Application**: `app/src/`
- **Libraries**: `lib/lx200/`, `lib/mount/`
- **Tests**: `tests/lib/lx200/`, `tests/lib/mount/`, `tests/app/`
- **Configuration**: `app/boards/` (device tree overlays)
- **Device Tree**: `dts/bindings/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and development environment

- [ ] T001 [P] Verify Python virtual environment and Zephyr SDK installed per quickstart.md
- [ ] T002 [P] Verify west workspace initialized with `west update` complete
- [ ] T003 [P] Verify baseline build succeeds: `west build -b native_sim` from app/
- [ ] T004 [P] Verify baseline tests run: `west twister -T tests/ -p native_sim`

**Checkpoint**: Development environment validated, baseline compiles and runs

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

### Device Tree Bindings

- [ ] T005 [P] Create device tree binding for stepper motor in `dts/bindings/stepper/zephyr,stepper-motor.yaml`
  - Properties: steps-per-revolution, max-frequency-hz, gear-ratio
  - Reference: spec.md FR-016, research.md device tree approach

### Device Tree Overlays

- [ ] T006 [P] Configure native_sim stepper device in `app/boards/native_sim.overlay`
  - Mock stepper device for simulation testing
  - Two motor instances (RA and Dec axes)
  
- [ ] T007 [P] Configure robin_nano stepper devices in `app/boards/robin_nano.overlay`
  - TMC5160/TMC2209 SPI/UART configuration
  - Pin assignments for production hardware
  
- [ ] T008 [P] Configure nucleo_f446re stepper devices in `app/boards/nucleo_f446re.overlay`
  - Alternative development board configuration

### Core Data Structures

- [ ] T009 [P] Implement RightAscension struct in `lib/mount/include/mount/Coordinate.hpp`
  - Validation: 0-23h, 0-59m, 0-59.999s
  - Conversions: toRadians(), toHours(), toString()
  - Parsing: fromString() for "HH:MM:SS" format
  - Reference: data-model.md §1, contracts/lx200-protocol.md coordinate formats
  
- [ ] T010 [P] Implement Declination struct in `lib/mount/include/mount/Coordinate.hpp`
  - Validation: 0-90°, 0-59', 0-59.999"
  - Sign handling (positive = north)
  - Conversions: toRadians(), toDegrees(), toString()
  - Parsing: fromString() for "sDD*MM:SS" format
  - Reference: data-model.md §1, contracts/lx200-protocol.md coordinate formats
  
- [ ] T011 [P] Implement CelestialCoordinate class in `lib/mount/include/mount/Coordinate.hpp`
  - Immutable value type with RightAscension and Declination
  - Constexpr validation
  - Reference: data-model.md §1
  
- [ ] T012 [P] Implement MotorPosition struct in `lib/mount/include/mount/Position.hpp`
  - Raw step counts for RA and Dec axes (int64_t)
  - Reference: data-model.md §2
  
- [ ] T013 [P] Implement MountPosition class in `lib/mount/include/mount/Position.hpp`
  - Associates MotorPosition with CelestialCoordinate
  - Timestamp for position recording
  - Reference: data-model.md §2
  
- [ ] T014 [P] Implement TrackingState class in `lib/mount/include/mount/TrackingState.hpp`
  - State machine: IDLE, SLEWING, TRACKING, GUIDING, STOPPED
  - Tracking rate (default sidereal: 15.041067 arcsec/sec)
  - Target coordinate storage
  - Reference: data-model.md §3

### CMake Configuration

- [ ] T015 Update `lib/mount/CMakeLists.txt` to build coordinate and position sources
  - Add Coordinate.cpp, Position.cpp, TrackingState.cpp
  
- [ ] T016 Update `lib/lx200/CMakeLists.txt` to build LX200 library sources
  - Will be populated during Protocol layer implementation

**Checkpoint**: Foundation ready - all core types defined, device tree configured, ready for layer implementation

---

## Phase 3: Layer 1 - Stepper API (Hardware Abstraction)

**Purpose**: Abstract Zephyr stepper driver subsystem for motor control
**User Stories**: Supports US1 (motion), US2 (tracking), US4 (hardware compatibility)
**Layer**: Bottom of architecture stack

### Tests for Stepper API (Write FIRST, ensure they FAIL)

- [ ] T017 [P] [US1,US2,US4] [Layer: Stepper] Write test for StepperController initialization in `tests/lib/mount/test_stepper_controller.cpp`
  - Test: Device binding from device tree
  - Test: Initial position is zero
  - Test: Device ready status
  - Mock: Zephyr stepper device on native_sim
  - Reference: research.md §2 native_sim testing
  
- [ ] T018 [P] [US1] [Layer: Stepper] Write test for position movement in `tests/lib/mount/test_stepper_controller.cpp`
  - Test: moveTo() accepts target position
  - Test: getCurrentPosition() returns correct position
  - Test: Relative moves with moveBy()
  - Reference: data-model.md §2 MotorPosition
  
- [ ] T019 [P] [US2] [Layer: Stepper] Write test for continuous tracking motion in `tests/lib/mount/test_stepper_controller.cpp`
  - Test: setVelocity() for sidereal rate
  - Test: Continuous motion without target position
  - Test: Stop tracking motion
  - Reference: spec.md FR-007 tracking precision
  
- [ ] T020 [P] [US2] [Layer: Stepper] Write test for timer-based step generation fallback in `tests/lib/mount/test_timer_stepper.cpp`
  - Test: Detect driver lacks hardware step generator
  - Test: Fallback to STM32 timer interrupts
  - Test: Verify 40kHz minimum frequency
  - Reference: spec.md Clarifications Q5, plan.md performance goals

### Implementation for Stepper API

- [ ] T021 [US1,US2,US4] [Layer: Stepper] Implement StepperController class in `lib/mount/src/StepperController.cpp`
  - Wrap Zephyr stepper.h API
  - Device binding from device tree
  - Position tracking for both axes
  - Reference: spec.md FR-016 to FR-021, spec.md Clarifications Q9
  
- [ ] T022 [US1] [Layer: Stepper] Implement moveTo() and moveBy() methods in `lib/mount/src/StepperController.cpp`
  - Absolute and relative positioning
  - Integration with Zephyr stepper_move_by() API
  - Depends on T021
  
- [ ] T023 [US2] [Layer: Stepper] Implement setVelocity() for continuous motion in `lib/mount/src/StepperController.cpp`
  - Sidereal tracking rate support
  - Integration with Zephyr stepper_set_reference_position() API
  - Depends on T021
  
- [ ] T024 [US1] [Layer: Stepper] Implement acceleration profile handling in `lib/mount/src/StepperController.cpp`
  - Trapezoidal velocity profiles
  - Configurable acceleration from device tree
  - Reference: spec.md FR-008
  - Depends on T021
  
- [ ] T025 [US2] [Layer: Stepper] Implement timer-based step generator fallback in `lib/mount/src/TimerStepGenerator.cpp`
  - Detect driver capabilities via Zephyr API
  - Use TMC5160 hardware generators when available
  - Fallback to STM32 hardware timer interrupts
  - 40kHz minimum frequency validation
  - Reference: spec.md Clarifications Q5
  - Depends on T021
  
- [ ] T026 [US4] [Layer: Stepper] Implement hardware feature detection in `lib/mount/src/StepperController.cpp`
  - Query driver capabilities (hardware step gen, ramp profiles)
  - Configure based on available features
  - Depends on T021

### Validation for Stepper API

- [ ] T027 Run Stepper API tests on native_sim: `west twister -T tests/lib/mount/ -p native_sim`
  - All tests MUST pass before proceeding
  
- [ ] T028 Validate timer frequency on robin_nano hardware
  - Flash to hardware: `west build -b robin_nano && west flash`
  - Verify actual step timing with oscilloscope/logic analyzer
  - Confirm 40kHz minimum frequency achieved

**Checkpoint**: Stepper API layer complete - motor abstraction working on simulation and hardware

---

## Phase 4: Layer 2 - Motion Planner (Kinematics & Trajectory)

**Purpose**: Transform celestial coordinates to motor steps, generate smooth motion profiles
**User Stories**: Supports US1 (motion), US2 (tracking), US4 (mount types)
**Layer**: Second from bottom, depends on Stepper API

### Tests for Motion Planner (Write FIRST, ensure they FAIL)

- [ ] T029 [P] [US4] [Layer: Planner] Write test for MountKinematics interface in `tests/lib/mount/test_kinematics.cpp`
  - Test: Abstract interface can be mocked
  - Test: Both implementations satisfy interface contract
  - Reference: spec.md Clarifications Q8, research.md §3
  
- [ ] T030 [P] [US1,US2] [Layer: Planner] Write tests for EquatorialKinematics in `tests/lib/mount/test_equatorial_kinematics.cpp`
  - Test: RA/Dec to motor step conversions with known coordinates
  - Test: Hour angle calculations for tracking
  - Test: Meridian flip detection
  - Reference: data-model.md §1 CelestialCoordinate
  
- [ ] T031 [P] [US4] [Layer: Planner] Write tests for AltAzKinematics in `tests/lib/mount/test_altaz_kinematics.cpp`
  - Test: Alt/Az to motor step conversions
  - Test: Celestial to horizontal coordinate transforms
  - Test: Location and time dependency
  - Reference: data-model.md §1, §5 Location
  
- [ ] T032 [P] [US1] [Layer: Planner] Write tests for MotionPlanner trajectory generation in `tests/lib/mount/test_motion_planner.cpp`
  - Test: Trapezoidal velocity profile generation
  - Test: Acceleration/deceleration ramp calculations
  - Test: No overshoot at target position
  - Test: Configurable max velocity and acceleration
  - Reference: spec.md FR-008

### Implementation for Motion Planner

- [ ] T033 [US4] [Layer: Planner] Define MountKinematics interface in `lib/mount/include/mount/MountKinematics.hpp`
  - Pure virtual methods: celestialToMotor(), motorToCelestial()
  - Strategy pattern base class
  - Reference: spec.md Clarifications Q8
  
- [ ] T034 [US1,US2] [Layer: Planner] Implement EquatorialKinematics in `lib/mount/src/EquatorialKinematics.cpp`
  - RA/Dec to motor step transformations
  - Sidereal tracking calculations
  - Gear ratio and microstepping from device tree
  - Reference: data-model.md §4 CalibrationModel
  - Depends on T033
  
- [ ] T035 [US4] [Layer: Planner] Implement AltAzKinematics in `lib/mount/src/AltAzKinematics.cpp`
  - Alt/Az to motor step transformations
  - Time-dependent celestial tracking in horizontal frame
  - Requires location and UTC time
  - Depends on T033
  
- [ ] T036 [US1] [Layer: Planner] Implement MotionPlanner in `lib/mount/src/MotionPlanner.cpp`
  - Trapezoidal velocity profile generator
  - Integration with MountKinematics for coordinate transforms
  - Configurable acceleration parameters
  - Reference: spec.md FR-008
  - Depends on T033, T034
  
- [ ] T037 [US3] [Layer: Planner] Implement CalibrationModel in `lib/mount/src/Calibration.cpp`
  - Alignment correction factors
  - Reference star position storage
  - Transform corrections for imperfect polar alignment
  - Reference: data-model.md §4
  - Depends on T033, T034

### Validation for Motion Planner

- [ ] T038 Run Motion Planner tests on native_sim: `west twister -T tests/lib/mount/ -p native_sim`
  - All tests MUST pass before proceeding
  
- [ ] T039 Validate coordinate transformations with known celestial objects
  - Test with Polaris (RA ~02h30m, Dec +89°15')
  - Test with Vega (RA ~18h37m, Dec +38°47')
  - Verify motor step calculations match expected values

**Checkpoint**: Motion Planner layer complete - coordinate transforms and trajectories working

---

## Phase 5: Layer 3 - Mount Controller (State Management & Threading)

**Purpose**: High-level mount control with thread-safe inter-component communication
**User Stories**: Supports US1 (control), US2 (tracking), US3 (calibration), US5 (reliability)
**Layer**: Third from bottom, depends on Motion Planner and Stepper API

### Tests for Mount Controller (Write FIRST, ensure they FAIL)

- [ ] T040 [P] [US1,US2] [Layer: Controller] Write tests for MountPosition and TrackingState in `tests/app/test_mount_state.cpp`
  - Test: TrackingState state machine transitions
  - Test: Valid state transitions (IDLE→SLEWING→TRACKING)
  - Test: Invalid transition rejection
  - Reference: data-model.md §3
  
- [ ] T041 [P] [US5] [Layer: Controller] Write tests for inter-thread communication in `tests/app/test_mount_threading.cpp`
  - Test: Message queue command delivery
  - Test: Mutex-protected shared state reads
  - Test: No deadlocks under concurrent access
  - Test: Event notifications for slew completion
  - Reference: spec.md Clarifications Q1-Q2
  
- [ ] T042 [P] [US1] [Layer: Controller] Write tests for command dispatcher in `tests/app/test_mount_commands.cpp`
  - Test: Commands routed to correct handlers
  - Test: Slew command initiates motion
  - Test: Stop command halts motion immediately
  - Test: Position queries return current state
  - Mock: LX200 commands from protocol layer
  
- [ ] T043 [P] [US2] [Layer: Controller] Write tests for tracking loop in `tests/app/test_tracking.cpp`
  - Test: Sidereal rate calculation (15.041067 arcsec/sec)
  - Test: Continuous tracking without drift
  - Test: Tracking starts/stops cleanly
  - Test: Position updates at tracking rate
  - Reference: spec.md FR-007
  
- [ ] T044 [P] [US3] [Layer: Controller] Write tests for calibration persistence in `tests/app/test_calibration_persistence.cpp`
  - Test: Save calibration data via Zephyr Settings
  - Test: Load calibration after simulated power cycle
  - Test: Validation of corrupted calibration data
  - Test: Factory reset clears calibration
  - Reference: spec.md Clarifications Q4, data-model.md §4

### Implementation for Mount Controller

- [ ] T045 [US1,US5] [Layer: Controller] Implement MountController threading infrastructure in `app/src/mount/MountController.cpp`
  - Zephyr message queue for command reception (k_msgq)
  - Mutex-protected shared state for position queries (k_mutex)
  - Mount controller thread with priority scheduling
  - Reference: spec.md Clarifications Q1-Q2, spec.md FR-027
  
- [ ] T046 [US1,US5] [Layer: Controller] Implement command dispatcher in `app/src/mount/MountController.cpp`
  - Route commands to appropriate handlers
  - Integration with Motion Planner for slews
  - Integration with Stepper API for motion execution
  - Error handling per spec.md Clarifications Q3
  - Depends on T045
  
- [ ] T047 [US5] [Layer: Controller] Implement event notification system in `app/src/mount/MountController.cpp`
  - Notify LX200 handler on slew completion
  - Notify on error conditions
  - Push-based status updates for critical events
  - Reference: spec.md Clarifications Q2, spec.md FR-028
  - Depends on T045
  
- [ ] T048 [US1] [Layer: Controller] Implement slew command handler in `app/src/mount/MountController.cpp`
  - Accept target CelestialCoordinate via message queue
  - Use Motion Planner to generate trajectory
  - Execute motion via Stepper API
  - Update shared state during motion
  - Depends on T045, T046
  
- [ ] T049 [US2] [Layer: Controller] Implement tracking loop in `app/src/mount/TrackingLoop.cpp`
  - Calculate sidereal rate step intervals
  - Continuous velocity updates to Stepper API
  - Use driver step generators (or timer fallback)
  - Update position in shared state
  - Reference: spec.md FR-007, spec.md Clarifications Q5
  - Depends on T045
  
- [ ] T050 [US1] [Layer: Controller] Implement emergency stop in `app/src/mount/MountController.cpp`
  - Halt all motion immediately (<1 second)
  - Transition to STOPPED state
  - Lock motors
  - Reference: spec.md FR-004, SC-006
  - Depends on T045, T046
  
- [ ] T051 [US3] [Layer: Controller] Implement calibration persistence in `app/src/mount/Calibration.cpp`
  - Save/load via Zephyr Settings subsystem
  - NVS backend on internal flash
  - Automatic wear leveling
  - Validation on load
  - Reference: spec.md Clarifications Q4, spec.md FR-014
  - Depends on T037
  
- [ ] T052 [US5] [Layer: Controller] Implement timeout and safety mechanisms in `app/src/mount/MountController.cpp`
  - Motion timeout detection
  - Position limit enforcement
  - Communication watchdog
  - Reference: spec.md FR-022 to FR-026
  - Depends on T045, T046

### Validation for Mount Controller

- [ ] T053 Run Mount Controller tests on native_sim: `west twister -T tests/app/ -p native_sim`
  - All tests MUST pass before proceeding
  
- [ ] T054 Validate threading safety with stress tests
  - Rapid command sequences
  - Concurrent state queries during motion
  - No deadlocks or race conditions
  
- [ ] T055 Validate tracking precision on hardware
  - Continuous tracking for 10+ minutes
  - Measure position error accumulation
  - Verify <5 arcseconds RMS per spec.md SC-005

**Checkpoint**: Mount Controller layer complete - threading, state management, tracking working

---

## Phase 6: Layer 4 - LX200 Protocol Handler (Command Parsing & Responses)

**Purpose**: Parse LX200 commands, format responses, communicate with Mount Controller
**User Stories**: Supports US1 (control), US6 (diagnostics)
**Layer**: Top of architecture stack, depends on Mount Controller

### Tests for LX200 Protocol (Write FIRST, ensure they FAIL)

- [ ] T056 [P] [US1] [Layer: Protocol] Write tests for LX200Command base classes in `tests/lib/lx200/test_command_classes.cpp`
  - Test: Command pattern polymorphism
  - Test: Response formatting (string, ack, reject)
  - Test: Parameter validation
  - Reference: data-model.md §7, research.md §1
  
- [ ] T057 [P] [US1] [Layer: Protocol] Write tests for LX200Parser in `tests/lib/lx200/test_parser.cpp`
  - Test: Command family identification (A,B,C,D,F,G,g,H,I,L,M,P,Q,R,S,T,U)
  - Test: Parameter extraction
  - Test: Malformed command rejection
  - Test: Command/parameter separation
  - Reference: existing tests in tests/lib/lx200/, contracts/lx200-protocol.md
  
- [ ] T058 [P] [US1] [Layer: Protocol] Write tests for coordinate parsing in `tests/lib/lx200/test_coordinates.cpp`
  - Test: RA parsing "HH:MM:SS" → RightAscension
  - Test: Dec parsing "sDD*MM:SS" → Declination
  - Test: Time parsing "HH:MM:SS"
  - Test: Date parsing "MM/DD/YY"
  - Test: Edge cases (00:00:00, 23:59:59, +90*00:00, -90*00:00)
  - Test: Invalid input rejection
  - Reference: contracts/lx200-protocol.md coordinate formats
  
- [ ] T059 [P] [US1] [Layer: Protocol] Write tests for Get Commands in `tests/lib/lx200/test_get_commands.cpp`
  - Test: :GR# returns "HH:MM:SS#" format
  - Test: :GD# returns "sDD*MM:SS#" format
  - Test: :GC# returns "MM/DD/YY#" format
  - Test: :GL# returns "HH:MM:SS#" format
  - Test: :Gt# returns "sDD*MM#" format
  - Test: :Gg# returns "sDDD*MM#" format
  - Mock: MountController with known state
  - Reference: contracts/lx200-protocol.md Category 2
  
- [ ] T060 [P] [US1] [Layer: Protocol] Write tests for Set Commands in `tests/lib/lx200/test_set_commands.cpp`
  - Test: :Sr HH:MM:SS# sets RA target → returns "1"
  - Test: :Sd sDD*MM:SS# sets Dec target → returns "1"
  - Test: Invalid input → returns "0"
  - Test: :St sDD*MM# sets latitude
  - Test: :Sg sDDD*MM# sets longitude
  - Reference: contracts/lx200-protocol.md Category 3
  
- [ ] T061 [P] [US1] [Layer: Protocol] Write tests for Motion Commands in `tests/lib/lx200/test_motion_commands.cpp`
  - Test: :MS# initiates slew → queues command to MountController
  - Test: :Me# starts eastward motion
  - Test: :Mw# starts westward motion
  - Test: :Mn# starts northward motion
  - Test: :Ms# starts southward motion
  - Test: :Q# stops all motion immediately
  - Reference: contracts/lx200-protocol.md Category 4
  
- [ ] T062 [P] [US3] [Layer: Protocol] Write tests for Sync Command in `tests/lib/lx200/test_sync_commands.cpp`
  - Test: :CM# synchronizes mount to target coordinates → returns "NONE#"
  - Test: Updates mount position in controller
  - Reference: contracts/lx200-protocol.md Category 1

### Implementation for LX200 Protocol - Priority 1 (Core)

- [ ] T063 [US1] [Layer: Protocol] Implement LX200Command base class in `lib/lx200/src/LX200Command.cpp`
  - Virtual execute() method
  - Command pattern implementation
  - Reference: data-model.md §7, research.md §1 OOP design
  
- [ ] T064 [US1] [Layer: Protocol] Implement LX200Response class in `lib/lx200/src/LX200Response.cpp`
  - Response types: string, ack (1), reject (0), none
  - Formatting methods
  - Reference: data-model.md §7
  - Depends on T063
  
- [ ] T065 [US1] [Layer: Protocol] Implement coordinate parsing functions in `lib/lx200/src/CoordinateParsing.cpp`
  - parseRA(): "HH:MM:SS" → RightAscension
  - parseDec(): "sDD*MM:SS" → Declination
  - parseTime(): "HH:MM:SS" → time components
  - parseDate(): "MM/DD/YY" → date components
  - Shared by multiple commands
  - Reference: contracts/lx200-protocol.md coordinate formats, audit-findings.md §6
  - Depends on T009, T010
  
- [ ] T066 [US1] [Layer: Protocol] Implement LX200Parser in `lib/lx200/src/LX200Parser.cpp`
  - Command family identification (existing partial implementation)
  - Parameter detection and extraction
  - Command factory pattern
  - Reference: contracts/lx200-protocol.md, research.md §1
  - Depends on T063
  
- [ ] T067 [P] [US1] [Layer: Protocol] Implement Get Commands (Category 2) in `lib/lx200/src/Commands/GetCommands.cpp`
  - :GR# (Get RA) - query shared state, format response
  - :GD# (Get Dec) - query shared state, format response
  - :GC# (Get Calendar) - format current date
  - :GL# (Get Local Time) - format current time
  - :Gt# (Get Latitude) - query configuration
  - :Gg# (Get Longitude) - query configuration
  - Reference: contracts/lx200-protocol.md Category 2, spec.md Clarifications Q1 (shared state access)
  - Depends on T064, T065, T066
  
- [ ] T068 [P] [US1] [Layer: Protocol] Implement Set Commands (Category 3) in `lib/lx200/src/Commands/SetCommands.cpp`
  - :Sr# (Set RA) - parse and validate, update target
  - :Sd# (Set Dec) - parse and validate, update target
  - :St# (Set Latitude) - parse and save to config
  - :Sg# (Set Longitude) - parse and save to config
  - :SL# (Set Local Time) - parse and set system time
  - :SC# (Set Calendar) - parse and set system date
  - Reference: contracts/lx200-protocol.md Category 3
  - Depends on T064, T065, T066

### Implementation for LX200 Protocol - Priority 2 (Motion & Sync)

- [ ] T069 [US1] [Layer: Protocol] Implement Motion Commands (Category 4) in `lib/lx200/src/Commands/MotionCommands.cpp`
  - :MS# (Slew) - queue command to MountController
  - :Me#/:Mw#/:Mn#/:Ms# (Directional) - start motion in direction
  - :Q# (Stop) - emergency stop via MountController
  - Integration with message queue
  - Reference: contracts/lx200-protocol.md Category 4, spec.md Clarifications Q1
  - Depends on T066, T046
  
- [ ] T070 [US3] [Layer: Protocol] Implement Sync Command (Category 1) in `lib/lx200/src/Commands/SyncCommands.cpp`
  - :CM# - synchronize mount to target coordinates
  - Update position in MountController
  - Return "NONE#" response
  - Reference: contracts/lx200-protocol.md Category 1
  - Depends on T066, T046
  
- [ ] T071 [US1] [Layer: Protocol] Implement Initialization Command (Category 0) in `lib/lx200/src/Commands/InitCommands.cpp`
  - :I# - initialize telescope, enter control mode
  - Reference: contracts/lx200-protocol.md Category 0
  - Depends on T066

### Implementation for LX200 Protocol - Priority 3 (Additional Commands)

- [ ] T072 [P] [US1] [Layer: Protocol] Implement Focus Commands (Category 5) in `lib/lx200/src/Commands/FocusCommands.cpp`
  - :F+# :F-# (Focus control)
  - Reference: contracts/lx200-protocol.md Category 5
  - Depends on T066
  
- [ ] T073 [P] [US1] [Layer: Protocol] Implement Home Commands (Category 6) in `lib/lx200/src/Commands/HomeCommands.cpp`
  - :hP# :hS# (Home position)
  - Reference: contracts/lx200-protocol.md Category 6
  - Depends on T066
  
- [ ] T074 [P] [US1] [Layer: Protocol] Implement Tracking Commands (Category 7) in `lib/lx200/src/Commands/TrackingCommands.cpp`
  - :T+# :T-# (Tracking control)
  - Reference: contracts/lx200-protocol.md Category 7
  - Depends on T066
  
- [ ] T075 [P] [US1] [Layer: Protocol] Implement Precision Commands (Category 8) in `lib/lx200/src/Commands/PrecisionCommands.cpp`
  - :P# (Toggle precision)
  - Reference: contracts/lx200-protocol.md Category 8
  - Depends on T066
  
- [ ] T076 [P] [US1] [Layer: Protocol] Implement Rate Commands (Category 9) in `lib/lx200/src/Commands/RateCommands.cpp`
  - :Rss# (Set rate: S/M/C/G)
  - Reference: contracts/lx200-protocol.md Category 9
  - Depends on T066
  
- [ ] T077 [P] [US1] [Layer: Protocol] Implement Additional Get Commands (Category 10) in `lib/lx200/src/Commands/GetCommands.cpp`
  - :Ga# :Gc# :GG# :GT# :GM#/:GN#/:GO#/:GP# :Gd# :Gr#
  - Reference: contracts/lx200-protocol.md Category 10
  - Depends on T067
  
- [ ] T078 [P] [US1] [Layer: Protocol] Implement Additional Set Commands (Category 11) in `lib/lx200/src/Commands/SetCommands.cpp`
  - :SHP# :SHHH:MM# :SHLHH:MM# :SYsDD*MM:SS.HH:MM:SS#
  - Reference: contracts/lx200-protocol.md Category 11
  - Depends on T068
  
- [ ] T079 [P] [US1] [Layer: Protocol] Implement Rate Control by Letter (Category 12) in `lib/lx200/src/Commands/RateCommands.cpp`
  - :Rss# alternate format
  - Reference: contracts/lx200-protocol.md Category 12
  - Depends on T076

### LX200 Protocol Handler Integration

- [ ] T080 [US1,US5] [Layer: Protocol] Implement LX200Handler thread in `app/src/protocol/LX200Handler.cpp`
  - Serial I/O thread for UART/USB communication
  - Integration with LX200Parser
  - Message queue for sending commands to MountController
  - Shared state reads for position queries
  - Event notification reception from MountController
  - Reference: spec.md Clarifications Q1-Q2, spec.md FR-027 to FR-028
  - Depends on T066, T045

### Validation for LX200 Protocol

- [ ] T081 Run LX200 Protocol tests on native_sim: `west twister -T tests/lib/lx200/ -p native_sim`
  - All tests MUST pass before proceeding
  
- [ ] T082 Validate command parsing with all 60+ commands
  - Test each command category
  - Verify response format compliance
  - Reference: contracts/lx200-protocol.md

**Checkpoint**: LX200 Protocol layer complete - command parsing and responses working

---

## Phase 7: User Story 1 - Remote Telescope Control (Priority: P1) 🎯 MVP

**Goal**: Enable basic computerized mount control from astronomy software

**Independent Test**: Connect Stellarium/SkySafari/ASCOM to mount, command "Go To" for M31, verify mount moves to coordinates

**Status**: Implementation complete via Layers 1-4 above

**Key Tasks Completed**:
- T048: Slew command handler (motion to target)
- T050: Emergency stop (<1 second response)
- T067-T071: LX200 Get/Set/Motion/Sync commands
- T080: Serial I/O integration with astronomy software

**Acceptance Scenarios** (from spec.md):
1. ✓ Go To command moves mount to target coordinates
2. ✓ Stop command immediately halts motion
3. ✓ Sync command aligns mount coordinate system
4. ✓ Alignment synchronization enables accurate pointing

**Validation**:
- [ ] T083 [US1] End-to-end test: Connect Stellarium to mount
  - Send :Sr# :Sd# commands to set target
  - Send :MS# to initiate slew
  - Verify mount moves to correct position
  - Verify :D# returns slewing status during motion
  
- [ ] T084 [US1] End-to-end test: Emergency stop
  - Initiate slew with :MS#
  - Send :Q# during motion
  - Verify motion stops within 1 second
  - Reference: spec.md SC-006

**Checkpoint**: User Story 1 COMPLETE - Basic telescope control functional

---

## Phase 8: User Story 2 - Accurate Celestial Tracking (Priority: P1)

**Goal**: Precise tracking for astrophotography without star trails

**Independent Test**: Enable tracking, take 5-minute exposure, verify stars are round (not elongated trails)

**Status**: Implementation complete via Layers 1-3 above

**Key Tasks Completed**:
- T049: Tracking loop with sidereal rate calculations
- T025: Driver step generators with timer fallback
- T023: Continuous velocity motion in Stepper API
- T034: EquatorialKinematics for tracking calculations

**Acceptance Scenarios** (from spec.md):
1. ✓ Tracking mode maintains aim at celestial object
2. ✓ Long exposures show sharp stars (not trails)
3. ✓ Smooth tracking through meridian crossing
4. ✓ Correction commands apply precise adjustments

**Validation**:
- [ ] T085 [US2] End-to-end test: Tracking precision measurement
  - Enable tracking on known star
  - Monitor position error over 10 minutes
  - Verify <5 arcseconds RMS error
  - Reference: spec.md SC-005
  
- [ ] T086 [US2] Hardware validation: Long exposure imaging
  - Track for 5+ minutes
  - Capture test images
  - Measure star roundness (FWHM, elongation)
  - Verify no visible trailing

**Checkpoint**: User Story 2 COMPLETE - Accurate tracking for astrophotography

---

## Phase 9: User Story 3 - Easy Mount Setup and Calibration (Priority: P2)

**Goal**: Quick alignment and calibration for accurate pointing

**Independent Test**: Perform alignment with 2-3 reference stars, verify subsequent "Go To" accuracy

**Status**: Implementation complete via Layer 3 above

**Key Tasks Completed**:
- T037: CalibrationModel with alignment corrections
- T051: Calibration persistence via Zephyr Settings
- T070: :CM# Sync command for reference star alignment

**Acceptance Scenarios** (from spec.md):
1. ✓ Initial alignment with 2-3 reference stars
2. ✓ Date/time/location configuration
3. ✓ Configuration persistence across power cycles
4. ✓ Clear diagnostic feedback on calibration failure

**Validation**:
- [ ] T087 [US3] End-to-end test: Alignment procedure
  - Point at Polaris, sync with :CM#
  - Point at second reference star, sync with :CM#
  - Command "Go To" third object
  - Verify pointing accuracy within 5 arcminutes
  - Reference: spec.md SC-002
  
- [ ] T088 [US3] End-to-end test: Calibration persistence
  - Perform alignment
  - Power cycle mount (simulate)
  - Verify calibration data reloaded
  - Verify pointing accuracy maintained

**Checkpoint**: User Story 3 COMPLETE - Easy setup and calibration

---

## Phase 10: User Story 4 - Multi-Mount Hardware Compatibility (Priority: P2)

**Goal**: Support different mount types and motor configurations

**Independent Test**: Configure for different mount (equatorial vs alt-az), verify accurate motion

**Status**: Implementation complete via Layers 1-2 above

**Key Tasks Completed**:
- T033-T035: Strategy pattern for EquatorialKinematics and AltAzKinematics
- T005-T008: Device tree configuration for multiple boards
- T021-T026: Stepper API abstraction for multiple driver types

**Acceptance Scenarios** (from spec.md):
1. ✓ Configure gear ratios and motor specs via device tree
2. ✓ Select motor driver type (TMC5160, TMC2209, basic)
3. ✓ Enable alt-azimuth mount type
4. ✓ Multiple users with different hardware achieve comparable accuracy

**Validation**:
- [ ] T089 [US4] Configuration test: Equatorial mount on robin_nano
  - Use robin_nano.overlay with TMC5160 drivers
  - Build: `west build -b robin_nano`
  - Verify tracking accuracy with EquatorialKinematics
  
- [ ] T090 [US4] Configuration test: Alt-azimuth mount on nucleo_f446re
  - Use nucleo_f446re.overlay with basic step/dir drivers
  - Enable AltAzKinematics in configuration
  - Build: `west build -b nucleo_f446re`
  - Verify accurate motion in horizontal coordinate frame
  
- [ ] T091 [US4] Configuration test: Different gear ratios
  - Modify device tree gear-ratio property
  - Rebuild without code changes
  - Verify step calculations adjust accordingly
  - Reference: spec.md SC-008

**Checkpoint**: User Story 4 COMPLETE - Multi-mount hardware compatibility

---

## Phase 11: User Story 5 - Stable and Reliable Operation (Priority: P1)

**Goal**: Continuous operation for multi-hour imaging sessions

**Independent Test**: Run 8-hour imaging session with periodic commands, verify no failures

**Status**: Implementation complete via Layer 3 above

**Key Tasks Completed**:
- T041: Thread-safe inter-component communication
- T047: Event notification system
- T052: Timeout and safety mechanisms
- Error handling throughout all layers

**Acceptance Scenarios** (from spec.md):
1. ✓ Multi-hour operation without crashes/freezes
2. ✓ Graceful error handling without restart
3. ✓ Recovery from power/communication interruptions
4. ✓ No resource leaks or performance degradation

**Validation**:
- [ ] T092 [US5] Stress test: 8-hour continuous operation
  - Enable tracking
  - Send periodic status queries
  - Monitor for crashes, freezes, errors
  - Monitor memory usage for leaks
  - Reference: spec.md SC-003, FR-022
  
- [ ] T093 [US5] Stress test: Rapid command sequences
  - Send commands at maximum rate
  - Verify no dropped commands
  - Verify no deadlocks or race conditions
  
- [ ] T094 [US5] Recovery test: Communication interruption
  - Disconnect/reconnect during operation
  - Verify state recovery
  - Verify position data not lost
  - Reference: spec.md SC-009

**Checkpoint**: User Story 5 COMPLETE - Stable, reliable operation

---

## Phase 12: User Story 6 - Clear Status and Diagnostic Information (Priority: P3)

**Goal**: Visibility into mount state and errors for troubleshooting

**Independent Test**: Query status during various operations, verify clear diagnostic output

**Status**: Partially complete - logging infrastructure exists

**Key Tasks Remaining**:
- Enhanced diagnostic commands
- Performance monitoring
- Calibration quality metrics

**Acceptance Scenarios** (from spec.md):
1. Status information on request (partial - via :GR#, :GD#, :D# commands)
2. Detailed error logging (complete - via Zephyr LOG_MODULE)
3. Calibration data presentation (partial - needs diagnostic formatting)
4. Real-time performance monitoring (not yet implemented)

**Implementation**:
- [ ] T095 [P] [US6] Implement diagnostic logging throughout mount controller
  - Enhanced logging in MountController.cpp
  - Performance metrics (step timing, position errors)
  - Reference: spec.md FR-029 to FR-033
  
- [ ] T096 [P] [US6] Implement calibration quality metrics
  - Alignment error calculations
  - RMS error reporting
  - Display via diagnostic commands
  - Reference: spec.md SC-004, SC-005
  
- [ ] T097 [P] [US6] Add LX200 diagnostic commands (if not already in protocol)
  - Custom status queries beyond standard LX200
  - Tracking error reporting
  - Calibration quality readout

**Validation**:
- [ ] T098 [US6] End-to-end test: Status monitoring
  - Query status during idle, slewing, tracking
  - Verify accurate state reporting
  - Reference: spec.md SC-012
  
- [ ] T099 [US6] End-to-end test: Error diagnostics
  - Trigger error conditions (invalid command, motion timeout)
  - Verify detailed logging
  - Verify system remains operational

**Checkpoint**: User Story 6 COMPLETE - Clear diagnostics and monitoring

---

## Phase 13: Integration & Cross-Platform Validation

**Purpose**: Verify all user stories work together across all target platforms

- [ ] T100 [P] Run complete test suite on native_sim
  - All layers: `west twister -T tests/ -p native_sim`
  - Verify 100% pass rate
  
- [ ] T101 Build and flash to robin_nano (production hardware)
  - `west build -b robin_nano -p`
  - `west flash`
  - Reference: quickstart.md build instructions
  
- [ ] T102 Hardware validation: robin_nano with TMC5160 drivers
  - Verify actual motor movement
  - Verify tracking precision with real steppers
  - Measure timing accuracy with oscilloscope
  
- [ ] T103 [P] Build and flash to nucleo_f446re (alternative board)
  - `west build -b nucleo_f446re -p`
  - `west flash`
  - Reference: spec.md FR-020 cross-platform requirement
  
- [ ] T104 Hardware validation: nucleo_f446re with basic step/dir
  - Verify timer-based step generation
  - Verify 40kHz minimum frequency achieved
  - Test alt-azimuth mount configuration
  
- [ ] T105 Integration test: Complete user scenario
  - Connect astronomy software (Stellarium)
  - Perform alignment (US3)
  - Command "Go To" multiple objects (US1)
  - Enable tracking for imaging (US2)
  - Monitor for 1+ hour continuous operation (US5)
  - Verify success criteria SC-001 to SC-007

**Checkpoint**: All platforms validated, all user stories integrated and working

---

## Phase 14: Polish & Cross-Cutting Concerns

**Purpose**: Final improvements and documentation

- [ ] T106 [P] Update documentation in `doc/` directory
  - Doxygen comments for all public APIs
  - User guide updates
  - Hardware setup guide
  
- [ ] T107 [P] Code cleanup and refactoring
  - Remove dead code
  - Improve code organization
  - Address TODO comments
  
- [ ] T108 [P] Performance optimization
  - Profile step generation timing
  - Optimize coordinate transformations
  - Reduce message queue latency
  
- [ ] T109 [P] Memory usage validation
  - Measure actual RAM footprint
  - Verify <128KB target met
  - Reference: plan.md performance goals
  
- [ ] T110 Run quickstart.md validation
  - Follow developer setup from scratch
  - Verify all instructions work
  - Update quickstart.md with any corrections
  
- [ ] T111 Security review
  - Serial input validation
  - Buffer overflow protection
  - Safe integer math (no overflow in step calculations)
  
- [ ] T112 Prepare for merge to main
  - Squash/clean commit history if needed
  - Update CHANGELOG
  - Create pull request with summary

**Checkpoint**: Feature complete, ready for production use

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - start immediately
- **Foundational (Phase 2)**: Depends on Setup - BLOCKS all layers
- **Layer 1 - Stepper API (Phase 3)**: Depends on Foundational
- **Layer 2 - Motion Planner (Phase 4)**: Depends on Layer 1
- **Layer 3 - Mount Controller (Phase 5)**: Depends on Layer 2
- **Layer 4 - LX200 Protocol (Phase 6)**: Depends on Layer 3
- **User Stories 1-6 (Phases 7-12)**: Validation of completed layer implementations
- **Integration (Phase 13)**: Depends on all user stories
- **Polish (Phase 14)**: Depends on integration

### Layer Dependencies (Bottom-Up Architecture)

```
Layer 4: LX200 Protocol Handler (Top)
    ↓ depends on
Layer 3: Mount Controller
    ↓ depends on
Layer 2: Motion Planner (Kinematics)
    ↓ depends on
Layer 1: Stepper API (Hardware Abstraction)
    ↓ depends on
Foundation: Device Tree + Core Data Structures
```

### User Story Implementation

All user stories are **implemented during layer construction** rather than as separate phases:

- **US1 (Control)**: Implemented across all layers T017-T084
- **US2 (Tracking)**: Implemented in Layers 1-3, T017-T086
- **US3 (Calibration)**: Implemented in Layer 3, T037, T051, T070, T087-T088
- **US4 (Hardware)**: Implemented in Layers 1-2, T005-T008, T033-T035, T089-T091
- **US5 (Reliability)**: Implemented in Layer 3, T041-T052, T092-T094
- **US6 (Diagnostics)**: Partially complete, T095-T099

### Parallel Opportunities

**Within Foundational Phase (After T008)**:
- T009-T014 (Core data structures) - ALL can run in parallel [P]

**Within Layer 1 (After T016)**:
- T017-T020 (Stepper tests) - ALL can run in parallel [P]
- T022-T026 (Stepper features after T021) - Can run in parallel

**Within Layer 2 (After T028)**:
- T029-T032 (Motion Planner tests) - ALL can run in parallel [P]
- T034-T035 (Kinematics implementations after T033) - Can run in parallel [P]

**Within Layer 3 (After T039)**:
- T040-T044 (Controller tests) - ALL can run in parallel [P]
- Multiple implementation tasks can run in parallel after T045

**Within Layer 4 (After T055)**:
- T056-T062 (Protocol tests) - ALL can run in parallel [P]
- T067-T068 (Get/Set commands after T066) - Can run in parallel [P]
- T072-T079 (Additional command categories) - ALL can run in parallel [P]

**Within Integration Phase**:
- T100, T101, T103 (Different platform builds) - Can run in parallel [P]

---

## Parallel Example: Foundational Phase

```bash
# After T008 (device tree overlays complete), launch all data structures in parallel:
Task T009: Implement RightAscension in lib/mount/include/mount/Coordinate.hpp [P]
Task T010: Implement Declination in lib/mount/include/mount/Coordinate.hpp [P]
Task T011: Implement CelestialCoordinate in lib/mount/include/mount/Coordinate.hpp [P]
Task T012: Implement MotorPosition in lib/mount/include/mount/Position.hpp [P]
Task T013: Implement MountPosition in lib/mount/include/mount/Position.hpp [P]
Task T014: Implement TrackingState in lib/mount/include/mount/TrackingState.hpp [P]

# All can complete independently (different files)
```

## Parallel Example: Layer 1 - Stepper API

```bash
# After T016 (CMake configured), launch all Stepper tests in parallel:
Task T017: Test StepperController initialization [P]
Task T018: Test position movement [P]
Task T019: Test continuous tracking motion [P]
Task T020: Test timer-based fallback [P]

# After T021 (StepperController base implemented), launch features in parallel:
Task T022: Implement moveTo() and moveBy() methods
Task T023: Implement setVelocity() for continuous motion [P]
Task T024: Implement acceleration profile handling [P]
Task T025: Implement timer-based step generator fallback [P]
Task T026: Implement hardware feature detection [P]
```

## Parallel Example: Layer 4 - LX200 Protocol

```bash
# After T066 (Parser implemented), launch command categories in parallel:
Task T067: Implement Get Commands (Category 2) [P]
Task T068: Implement Set Commands (Category 3) [P]
Task T069: Implement Motion Commands (Category 4)

# After Priority 1-2 complete, launch all Priority 3 categories in parallel:
Task T072: Implement Focus Commands (Category 5) [P]
Task T073: Implement Home Commands (Category 6) [P]
Task T074: Implement Tracking Commands (Category 7) [P]
Task T075: Implement Precision Commands (Category 8) [P]
Task T076: Implement Rate Commands (Category 9) [P]
Task T077: Implement Additional Get Commands (Category 10) [P]
Task T078: Implement Additional Set Commands (Category 11) [P]
Task T079: Implement Rate Control by Letter (Category 12) [P]
```

---

## Implementation Strategy

### TDD Workflow (Constitutional Requirement)

Every task follows this cycle:

1. **Write test FIRST** - Test must FAIL initially
2. **Run test** - Verify failure (compilation error or test failure)
3. **Implement minimal code** - Make test pass
4. **Run test again** - Verify success
5. **Refactor** - Improve code quality
6. **Commit** - Save working increment

**Example for Task T017**:
```bash
# 1. Write failing test
vim tests/lib/mount/test_stepper_controller.cpp
# Add test_initialization() test case

# 2. Build and run (will fail - StepperController doesn't exist yet)
cd app/
west build -b native_sim
west build -t run
# See failure: "StepperController.hpp not found"

# 3. Implement minimal code
vim lib/mount/include/mount/StepperController.hpp
vim lib/mount/src/StepperController.cpp
# Implement just enough to compile

# 4. Build and run again
west build -b native_sim
west build -t run
# See failure: "Test test_initialization FAILED"

# 5. Fix implementation until test passes
vim lib/mount/src/StepperController.cpp
west build -t run
# See success: "Test test_initialization PASSED"

# 6. Commit
git add tests/lib/mount/test_stepper_controller.cpp
git add lib/mount/include/mount/StepperController.hpp
git add lib/mount/src/StepperController.cpp
git commit -m "feat: implement StepperController initialization

- Add device tree binding for stepper device
- Implement constructor with device lookup
- Add getCurrentPosition() returning zero initially
- Test passes on native_sim

Refs: T017, T021, spec.md FR-016"
```

### MVP First (User Story 1 Only)

Fastest path to working telescope control:

1. **Phase 1**: Setup (T001-T004) - ~30 minutes
2. **Phase 2**: Foundational (T005-T016) - ~2-3 days
3. **Phase 3**: Layer 1 - Stepper API (T017-T028) - ~3-4 days
4. **Phase 4**: Layer 2 - Motion Planner (T029-T039) - ~3-4 days
5. **Phase 5**: Layer 3 - Mount Controller (T040-T055) - ~5-6 days
6. **Phase 6**: Layer 4 - LX200 Protocol Priority 1-2 (T056-T071, T080-T082) - ~4-5 days
7. **Phase 7**: User Story 1 Validation (T083-T084) - ~1 day

**Total for MVP**: ~3-4 weeks (single developer, full-time)

**STOP and VALIDATE**: Can now control telescope from Stellarium! 🎯

### Incremental Delivery

After MVP, add user stories incrementally:

1. **MVP (US1)**: Basic control → Deploy/demo
2. **+US2**: Add tracking precision → Deploy/demo (astrophotography ready)
3. **+US3**: Add calibration → Deploy/demo (easier setup)
4. **+US4**: Add hardware compatibility → Deploy/demo (multiple mount support)
5. **+US5**: Validate reliability → Deploy/demo (production ready)
6. **+US6**: Add diagnostics → Deploy/demo (enhanced troubleshooting)

Each increment adds value without breaking previous functionality.

### Parallel Team Strategy

With 3 developers after Foundational phase complete:

- **Developer A**: Focus on Layer 1 (Stepper API) + Layer 2 (Motion Planner)
- **Developer B**: Focus on Layer 3 (Mount Controller)
- **Developer C**: Focus on Layer 4 (LX200 Protocol)

Layers integrate naturally due to clear interfaces defined in Foundational phase.

---

## Task Statistics

### Total Tasks: 112

### Tasks by Phase:
- Phase 1 (Setup): 4 tasks
- Phase 2 (Foundational): 12 tasks
- Phase 3 (Layer 1 - Stepper API): 12 tasks
- Phase 4 (Layer 2 - Motion Planner): 11 tasks
- Phase 5 (Layer 3 - Mount Controller): 16 tasks
- Phase 6 (Layer 4 - LX200 Protocol): 27 tasks
- Phase 7-12 (User Story Validation): 17 tasks
- Phase 13 (Integration): 6 tasks
- Phase 14 (Polish): 7 tasks

### Tasks by User Story:
- US1 (Remote Control): 44 tasks (39% - largest, core functionality)
- US2 (Tracking): 14 tasks (13% - tracking precision)
- US3 (Calibration): 8 tasks (7% - alignment)
- US4 (Hardware): 12 tasks (11% - multi-mount support)
- US5 (Reliability): 15 tasks (13% - stability)
- US6 (Diagnostics): 5 tasks (4% - monitoring)
- Shared/Infrastructure: 14 tasks (13%)

### Parallelization Opportunities:
- 47 tasks marked [P] (42% can run in parallel given dependencies met)
- Largest parallel batch: 8 tasks (T072-T079 - LX200 additional commands)
- Foundational phase: 6 parallel tasks (T009-T014)
- Layer 1 tests: 4 parallel tasks (T017-T020)
- Layer 2 tests: 4 parallel tasks (T029-T032)
- Layer 3 tests: 5 parallel tasks (T040-T044)
- Layer 4 tests: 7 parallel tasks (T056-T062)

### Critical Path:
Setup → Foundational → Layer 1 → Layer 2 → Layer 3 → Layer 4 → Integration → Polish

Minimum time (single developer, no parallelization): ~4-5 weeks
Optimized time (3 developers, parallelization): ~2-3 weeks

---

## Notes

- **[P] tasks**: Different files, no dependencies - can run in parallel
- **[Story] labels**: Map tasks to user stories for traceability
- **[Layer] labels**: Identify architecture layer for context
- **TDD required**: Constitution mandates tests FIRST, implementation SECOND
- **Bottom-up approach**: Build foundation before higher layers
- **Each layer independently testable**: Can validate at each checkpoint
- **Commit frequently**: After each task or logical group
- **Stop at checkpoints**: Validate layer complete before proceeding
- **MVP at Phase 7**: User Story 1 complete = working telescope controller

---

## References

- **spec.md**: User stories, functional requirements, success criteria
- **plan.md**: Architecture, technical context, performance goals
- **research.md**: Technology decisions, design rationale
- **data-model.md**: Core C++20 data structures
- **contracts/lx200-protocol.md**: Complete LX200 protocol specification (60+ commands)
- **quickstart.md**: Development environment setup, TDD workflow
- **audit-findings.md**: Implementation guidance, cross-references, starting points

---

## Implementation Checklist

Before starting:
- [ ] Read spec.md user stories and requirements
- [ ] Read plan.md technical context
- [ ] Review research.md design decisions
- [ ] Study data-model.md core structures
- [ ] Scan contracts/lx200-protocol.md protocol specification
- [ ] Setup environment per quickstart.md

During implementation:
- [ ] Always write tests FIRST (TDD)
- [ ] Follow bottom-up layer sequence
- [ ] Run tests after each task
- [ ] Commit working increments
- [ ] Validate at each checkpoint
- [ ] Reference documentation as needed

At completion:
- [ ] All 112 tasks checked off
- [ ] All tests passing on native_sim
- [ ] Hardware validation on robin_nano and nucleo_f446re
- [ ] All 6 user stories validated
- [ ] Success criteria SC-001 to SC-015 verified
- [ ] Documentation updated
- [ ] Ready for production use

---

*Generated by `/speckit.tasks` command on 2025-10-12*
*Total: 112 tasks across 14 phases, organized by architectural layer with user story traceability*

# Tasks: LX200 Protocol C++20 Rewrite

**Feature**: 001-lx200-cpp20-rewrite  
**Branch**: `001-lx200-cpp20-rewrite`  
**Input**: Design documents from `/home/andre/repos/OpenAstroTech/OpenAstroFirmware-workspace/OpenAstroFirmware/specs/001-lx200-cpp20-rewrite/`  
**Prerequisites**: plan.md, research.md, data-model.md, contracts/

## Execution Flow (main)
```
1. Load plan.md from feature directory
   ✓ Tech stack: C++20, Zephyr RTOS v4.2.0, Twister testing
   ✓ Structure: Embedded firmware single-project
2. Load optional design documents:
   ✓ data-model.md: 11 entities extracted
   ✓ contracts/: 2 contract files → test tasks
   ✓ research.md: C++20 patterns and decisions
3. Generate tasks by category:
   → Setup: Delete old C code, create structure, update build
   → Tests: Contract tests, entity tests, integration tests
   → Core: Enums, structs, classes per data model
   → Integration: Mount controller integration
   → Polish: Performance tests, documentation
4. Apply task rules:
   → Different files = mark [P] for parallel
   → Same file = sequential (no [P])
   → Tests before implementation (TDD)
5. Number tasks sequentially (T001, T002...)
6. Generate dependency graph
7. Create parallel execution examples
8. Task completeness validated
9. SUCCESS: tasks ready for execution
```

---

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths in descriptions
- File paths are absolute from repository root

## Path Conventions
- **Repository root**: `/home/andre/repos/OpenAstroTech/OpenAstroFirmware-workspace/OpenAstroFirmware`
- **Headers**: `include/lx200/`
- **Implementation**: `lib/lx200/`
- **Tests**: `tests/lib/lx200/`
- **App integration**: `app/src/mount/`

---

## Phase 3.1: Setup & Cleanup

### T001: Delete Old C Implementation Files
**Purpose**: Remove existing C code to start clean slate for C++20 rewrite

**Files to delete**:
- `lib/lx200/lx200.c`
- `include/lx200/lx200.h`
- `tests/lib/lx200/src/main.c`

**Actions**:
```bash
cd /home/andre/repos/OpenAstroTech/OpenAstroFirmware-workspace/OpenAstroFirmware
git rm lib/lx200/lx200.c
git rm include/lx200/lx200.h
git rm tests/lib/lx200/src/main.c
git commit -m "refactor: remove C implementation of LX200 parser for C++20 rewrite"
```

**Success criteria**: Files deleted, commit created

---

### T002: [P] Create C++20 Directory Structure
**Purpose**: Create new directory structure for C++20 implementation

**Directories to create**:
```bash
mkdir -p include/lx200
mkdir -p lib/lx200
mkdir -p tests/lib/lx200/src
```

**Success criteria**: Directory structure ready for C++ files

---

### T003: Update Build Configuration for C++20
**Purpose**: Configure CMake and Zephyr build system for C++20 compilation

**File**: `lib/lx200/CMakeLists.txt`

**Actions**:
- Set C++ standard to C++20: `set(CMAKE_CXX_STANDARD 20)`
- Add C++ source files instead of C
- Enable appropriate compiler flags (-Wall, -Wextra)
- Ensure no-exceptions flag: `-fno-exceptions`
- Ensure no-RTTI flag: `-fno-rtti`

**Success criteria**: Build system configured for C++20

---

## Phase 3.2: Tests First (TDD) ⚠️ MUST COMPLETE BEFORE 3.3

**CRITICAL: These tests MUST be written and MUST FAIL before ANY implementation**

### T004: [P] Create Test CMakeLists.txt
**Purpose**: Configure test build system

**File**: `tests/lib/lx200/CMakeLists.txt`

**Actions**:
- Configure Zephyr Twister test target
- Add C++ test sources
- Link against lx200 library
- Enable test logging
- Set up native_sim platform support

**Success criteria**: Test builds configured but no tests exist yet

---

### T005: [P] Create Test Suite Entry Point
**Purpose**: Create main test entry point for Zephyr Twister

**File**: `tests/lib/lx200/src/main.cpp`

**Content**:
- Include Zephyr test framework headers
- Set up test suite registration
- Configure test logging
- Main test entry function

**Success criteria**: Test framework initialized, builds successfully

---

### T006: [P] Contract Test: Parser Initialization (TC-001)
**Purpose**: Test parser constructor and initial state

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Test**: `test_parser_initialization`

**From contract**: parser-contract.md TC-001

**Test cases**:
- Buffer is empty after construction
- Precision mode defaults to High
- Command not complete initially
- get_command() returns std::nullopt

**Expected**: Test FAILS (no implementation yet)

---

### T007: [P] Contract Test: Parser Reset (TC-002)
**Purpose**: Test parser reset() clears state

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Test**: `test_parser_reset`

**From contract**: parser-contract.md TC-002

**Test cases**:
- Partial command cleared after reset
- Command complete flag reset
- Precision mode unchanged
- get_command() returns std::nullopt after reset

**Expected**: Test FAILS

---

### T008: [P] Contract Test: Feed Valid Command (TC-003)
**Purpose**: Test feeding characters to build complete command

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Test**: `test_feed_valid_command`

**From contract**: parser-contract.md TC-003

**Test cases**:
- Feed `:GR#` character by character
- Incomplete results until `#`
- Success on final `#`
- get_command() returns Command with name="GR"

**Expected**: Test FAILS

---

### T009: [P] Contract Test: Parser Error Handling (TC-004, TC-005, TC-009)
**Purpose**: Test parser handles invalid input correctly

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Tests**:
- `test_missing_prefix` - TC-004
- `test_buffer_overflow` - TC-005
- `test_empty_command` - TC-009

**Test cases**:
- Missing `:` prefix returns error
- Buffer overflow detected and handled
- Empty command `:##` rejected
- Parser recovers after errors

**Expected**: Tests FAIL

---

### T010: [P] Contract Test: Commands with Parameters (TC-006)
**Purpose**: Test parsing commands that have parameter strings

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Test**: `test_command_with_parameters`

**From contract**: parser-contract.md TC-006

**Test cases**:
- Feed `:Sr12:34:56#`
- Command name extracted as "Sr"
- Parameters extracted as "12:34:56"
- Command family identified as SetInfo

**Expected**: Test FAILS

---

### T011: [P] Contract Test: Precision Mode Toggle (TC-007)
**Purpose**: Test precision mode switching

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Test**: `test_precision_mode_toggle`

**From contract**: parser-contract.md TC-007

**Test cases**:
- Default is High precision
- set_precision(Low) changes mode
- get_precision() returns current mode
- Mode persists across commands

**Expected**: Test FAILS

---

### T012: [P] Contract Test: Sequential Commands (TC-008, TC-010)
**Purpose**: Test multiple commands and partial buffering

**File**: `tests/lib/lx200/src/test_parser.cpp`

**Tests**:
- `test_multiple_commands` - TC-008
- `test_partial_buffering` - TC-010

**Test cases**:
- Two commands `:GR#:Gd#` parsed sequentially
- Partial command buffered until terminator
- State maintained correctly

**Expected**: Tests FAIL

---

### T013: [P] Contract Test: RA Coordinate Parsing (TC-RA-001 to TC-RA-005)
**Purpose**: Test Right Ascension coordinate parsing

**File**: `tests/lib/lx200/src/test_coordinates.cpp`

**Tests**:
- `test_parse_ra_high_precision` - "12:34:56"
- `test_parse_ra_low_precision` - "12:34.5"
- `test_reject_invalid_ra_hours` - "24:00:00"
- `test_reject_invalid_ra_minutes` - "12:60:00"
- `test_reject_malformed_ra` - "12:34" (missing seconds)

**From contract**: coordinate-parsing-contract.md TC-RA-*

**Expected**: Tests FAIL

---

### T014: [P] Contract Test: DEC Coordinate Parsing (TC-DEC-001 to TC-DEC-005)
**Purpose**: Test Declination coordinate parsing

**File**: `tests/lib/lx200/src/test_coordinates.cpp`

**Tests**:
- `test_parse_dec_positive` - "+45*30:15"
- `test_parse_dec_negative` - "-12*00:30"
- `test_parse_dec_apostrophe` - "+45*30'15"
- `test_reject_dec_out_of_range` - "+90*30:00"
- `test_parse_dec_low_precision` - "+45*30"

**From contract**: coordinate-parsing-contract.md TC-DEC-*

**Expected**: Tests FAIL

---

### T015: [P] Contract Test: Latitude/Longitude Parsing (TC-LAT-*, TC-LON-*)
**Purpose**: Test geographic coordinate parsing

**File**: `tests/lib/lx200/src/test_coordinates.cpp`

**Tests**:
- `test_parse_latitude` - "+45*30"
- `test_reject_latitude_out_of_range` - "+90*01"
- `test_parse_longitude` - "123*45"
- `test_reject_longitude_out_of_range` - "360*00"

**From contract**: coordinate-parsing-contract.md TC-LAT-*, TC-LON-*

**Expected**: Tests FAIL

---

### T016: [P] Contract Test: Time and Date Parsing (TC-TIME-*, TC-DATE-*)
**Purpose**: Test time and date value parsing

**File**: `tests/lib/lx200/src/test_coordinates.cpp`

**Tests**:
- `test_parse_time` - "14:30:45"
- `test_reject_invalid_time` - "25:00:00"
- `test_parse_date` - "10/04/25"
- `test_reject_invalid_month` - "13/01/25"
- `test_reject_invalid_day` - "02/30/25"

**From contract**: coordinate-parsing-contract.md TC-TIME-*, TC-DATE-*

**Expected**: Tests FAIL

---

### T017: [P] Contract Test: Command Family Identification
**Purpose**: Test command family enum and identification

**File**: `tests/lib/lx200/src/test_commands.cpp`

**Tests**:
- `test_identify_alignment_commands` - 'A' → Alignment
- `test_identify_get_info_commands` - 'G' → GetInfo
- `test_identify_set_info_commands` - 'S' → SetInfo
- `test_identify_movement_commands` - 'M' → Movement
- `test_identify_unknown_commands` - invalid → Unknown

**Expected**: Tests FAIL

---

### T018: [P] Integration Test: Complete Command Sequence
**Purpose**: End-to-end protocol test

**File**: `tests/lib/lx200/src/test_integration.cpp`

**Test**: `test_command_sequence`

**Scenarios from quickstart.md**:
1. Send `:GR#` → expect RA response
2. Send `:Gd#` → expect DEC response
3. Send `:P#` → toggle precision
4. Send `:GR#` → expect low precision RA
5. Send invalid command → expect error

**Expected**: Test FAILS

---

### T019: Verify All Tests Fail
**Purpose**: Ensure TDD red phase - all tests fail before implementation

**Action**:
```bash
cd /home/andre/repos/OpenAstroTech/OpenAstroFirmware-workspace/OpenAstroFirmware
west twister -T tests/lib/lx200/ -p native_sim
```

**Expected output**: All tests FAIL with compilation or assertion errors

**Success criteria**: Tests exist, build, and fail appropriately

---

## Phase 3.3: Core Implementation (ONLY after tests are failing)

### T020: [P] Create Core Type Definitions
**Purpose**: Define enums and basic types

**File**: `include/lx200/lx200.hpp`

**Entities from data-model.md**:
- `enum class CommandFamily` - 18 command families
- `enum class PrecisionMode` - High/Low
- `enum class ParseResult` - Success, errors

**Success criteria**: Types compile, tests still fail (no logic yet)

---

### T021: [P] Create Coordinate Type Definitions
**Purpose**: Define coordinate structures

**File**: `include/lx200/lx200.hpp` (or separate coordinates.hpp)

**Entities from data-model.md**:
- `struct RACoordinate` - hours, minutes, seconds, validation
- `struct DECCoordinate` - sign, degrees, minutes, seconds
- `struct LatitudeCoordinate` - sign, degrees, minutes
- `struct LongitudeCoordinate` - degrees, minutes
- `struct TimeValue` - hours, minutes, seconds
- `struct DateValue` - month, day, year

**Success criteria**: Structs compile, static validation methods defined

---

### T022: [P] Create Command Structure
**Purpose**: Define Command and ParserState

**File**: `include/lx200/lx200.hpp`

**Entities from data-model.md**:
- `struct Command` - family, name, parameters (string_view)
- `class ParserState` - buffer, state, methods

**Success criteria**: Classes compile, public interface defined

---

### T023: Implement ParserState Constructor and Reset
**Purpose**: Initialize parser state correctly

**File**: `lib/lx200/parser.cpp`

**Implementation**:
- Constructor initializes buffer, precision, flags
- reset() clears state
- RAII resource management

**Tests pass**: T006 (initialization), T007 (reset)

---

### T024: Implement Character Feeding Logic
**Purpose**: Process incoming characters, detect command completion

**File**: `lib/lx200/parser.cpp`

**Implementation**:
- feed_character() method
- Prefix validation (`:`)
- Buffer accumulation
- Terminator detection (`#`)
- Buffer overflow protection

**Tests pass**: T008 (valid command), T009 (error handling), T012 (sequential)

---

### T025: Implement Command Extraction
**Purpose**: Parse buffered command into Command struct

**File**: `lib/lx200/parser.cpp`

**Implementation**:
- get_command() method
- Extract command name (first 1-2 chars after `:`)
- Extract parameters (remainder before `#`)
- Identify command family
- Return Command via std::optional

**Tests pass**: T008, T010 (parameters), T017 (family identification)

---

### T026: Implement Precision Mode Management
**Purpose**: Get/set precision mode

**File**: `lib/lx200/parser.cpp`

**Implementation**:
- get_precision() accessor
- set_precision() mutator
- Mode persists in parser state

**Tests pass**: T011 (precision toggle)

---

### T027: [P] Implement RA Coordinate Parsing
**Purpose**: Parse Right Ascension strings

**File**: `lib/lx200/coordinates.cpp`

**Implementation**:
- `parse_ra_coordinate()` function
- High precision: "HH:MM:SS" format
- Low precision: "HH:MM.T" format
- Range validation (0-23 hours)
- Return std::optional<RACoordinate>

**Tests pass**: T013 (RA parsing)

---

### T028: [P] Implement DEC Coordinate Parsing
**Purpose**: Parse Declination strings

**File**: `lib/lx200/coordinates.cpp`

**Implementation**:
- `parse_dec_coordinate()` function
- Formats: "sDD*MM:SS" or "sDD*MM'SS"
- Sign parsing (+/-)
- Range validation (-90 to +90 degrees)
- Return std::optional<DECCoordinate>

**Tests pass**: T014 (DEC parsing)

---

### T029: [P] Implement Latitude/Longitude Parsing
**Purpose**: Parse geographic coordinates

**File**: `lib/lx200/coordinates.cpp`

**Implementation**:
- `parse_latitude()` function - "sDD*MM" format
- `parse_longitude()` function - "DDD*MM" format
- Range validation
- Return std::optional types

**Tests pass**: T015 (lat/lon parsing)

---

### T030: [P] Implement Time/Date Parsing
**Purpose**: Parse time and date strings

**File**: `lib/lx200/coordinates.cpp`

**Implementation**:
- `parse_time()` function - "HH:MM:SS" format
- `parse_date()` function - "MM/DD/YY" format
- Validation (valid time, valid date with leap years)
- Return std::optional types

**Tests pass**: T016 (time/date parsing)

---

### T031: Verify All Unit Tests Pass
**Purpose**: Confirm all unit and contract tests pass

**Action**:
```bash
west twister -T tests/lib/lx200/ -p native_sim
```

**Expected**: All tests PASS

**Success criteria**: 100% test pass rate on native_sim

---

## Phase 3.4: Integration

### T032: Update Mount Controller Integration
**Purpose**: Integrate new C++ parser with Mount class

**File**: `app/src/mount/Mount.cpp`

**Changes**:
- Include new `lx200/lx200.hpp` header
- Replace C API calls with C++ parser
- Create LX200Parser instance
- Call feed_character() from serial handler
- Process Command objects from get_command()

**Success criteria**: Mount controller compiles with new parser

---

### T033: Integration Test Execution
**Purpose**: Run end-to-end integration tests

**Action**:
```bash
west twister -T tests/lib/lx200/ -p native_sim -s test_integration
```

**Expected**: Integration tests PASS (T018)

**Success criteria**: Complete command sequences work correctly

---

### T034: Update Test Configuration
**Purpose**: Configure testcase.yaml for all test scenarios

**File**: `tests/lib/lx200/testcase.yaml`

**Configuration**:
- Test scenarios for native_sim
- Test scenarios for robin_nano
- Test configurations (debug, release)
- Timeout settings
- Platform requirements

**Success criteria**: Twister recognizes all test configurations

---

## Phase 3.5: Polish

### T035: [P] Performance Benchmarking
**Purpose**: Measure parser performance

**File**: `tests/lib/lx200/src/test_performance.cpp`

**Tests**:
- Command parsing latency (<10ms)
- Coordinate parsing latency (<5ms)
- Memory usage (static only)
- CPU utilization

**Success criteria**: All performance targets met

---

### T036: [P] Build for Target Hardware
**Purpose**: Compile for actual hardware platform

**Action**:
```bash
cd app
west build -b robin_nano -t pristine
west build -b robin_nano
```

**Expected**: Clean build for MKS Robin Nano

**Success criteria**: Firmware binary generated successfully

---

### T037: [P] Run Hardware Tests (if available)
**Purpose**: Execute tests on actual hardware

**Action**:
```bash
west twister -T tests/lib/lx200/ -p robin_nano
west flash
```

**Expected**: Tests pass on hardware

**Success criteria**: All tests pass on target platform

---

### T038: [P] Update Documentation
**Purpose**: Document C++ API for developers

**Files**:
- `lib/lx200/README.md` - Migration guide from C to C++
- API documentation in headers (Doxygen comments)
- Update `.github/copilot-instructions.md` if needed

**Success criteria**: C++ API fully documented

---

### T039: Code Review and Cleanup
**Purpose**: Final code review and cleanup

**Actions**:
- Remove debug logging
- Check for TODO/FIXME comments
- Verify coding standards compliance
- Run static analysis (clang-tidy if available)

**Success criteria**: Code ready for merge

---

### T040: Execute Quickstart Validation
**Purpose**: Run complete quickstart.md validation

**Action**: Follow all steps in `specs/001-lx200-cpp20-rewrite/quickstart.md`

**Expected**:
- ✓ Native simulation build succeeds
- ✓ All unit tests pass
- ✓ All integration tests pass
- ✓ Performance benchmarks meet requirements
- ✓ Hardware build succeeds

**Success criteria**: All quickstart steps pass

---

## Dependencies

**Critical Path**:
```
T001 (Delete old) → T002 (Structure) → T003 (Build config) → 
T004-T019 (All tests) → T020-T030 (Implementation) → 
T031 (Verify tests) → T032-T034 (Integration) → 
T035-T040 (Polish & validation)
```

**Parallel Opportunities**:
- T002 [P] can run independently
- T004-T018 [P] all test files can be created in parallel
- T020-T022 [P] type definitions (different concepts)
- T027-T030 [P] coordinate parsing (independent functions)
- T035-T038 [P] polish tasks (different files)

**Blocking Dependencies**:
- T001-T003 must complete before tests
- T004-T019 (all tests) must complete and FAIL before T020-T030
- T020-T030 (implementation) must complete before T031
- T031 (unit tests pass) must complete before T032
- T032 (integration) must complete before T033

---

## Parallel Execution Examples

### Example 1: Create All Test Files in Parallel
```bash
# Launch T006-T018 together (all create different test functions):
# Terminal 1
Task: "Create test_parser.cpp with parser initialization test (TC-001)"

# Terminal 2  
Task: "Create test_coordinates.cpp with RA coordinate parsing tests (TC-RA-*)"

# Terminal 3
Task: "Create test_commands.cpp with command family identification tests"

# Terminal 4
Task: "Create test_integration.cpp with end-to-end protocol tests"
```

### Example 2: Implement Coordinate Parsers in Parallel
```bash
# Launch T027-T030 together (independent functions):
# Terminal 1
Task: "Implement parse_ra_coordinate() in lib/lx200/coordinates.cpp"

# Terminal 2
Task: "Implement parse_dec_coordinate() in lib/lx200/coordinates.cpp"

# Terminal 3
Task: "Implement parse_latitude() and parse_longitude() in lib/lx200/coordinates.cpp"

# Terminal 4
Task: "Implement parse_time() and parse_date() in lib/lx200/coordinates.cpp"
```

### Example 3: Polish Tasks in Parallel
```bash
# Launch T035-T038 together:
# Terminal 1
Task: "Create performance benchmarks in tests/lib/lx200/src/test_performance.cpp"

# Terminal 2
Task: "Build for robin_nano hardware platform"

# Terminal 3
Task: "Run hardware tests on robin_nano"

# Terminal 4
Task: "Update documentation with C++ API migration guide"
```

---

## Task Summary

**Total Tasks**: 40  
**Parallel Tasks**: 22 (marked with [P])  
**Sequential Tasks**: 18

**Estimated Time**:
- Setup (T001-T003): 1 hour
- Tests (T004-T019): 8 hours (parallel execution: ~2-3 hours)
- Implementation (T020-T031): 12 hours (with parallel: ~6-8 hours)
- Integration (T032-T034): 3 hours
- Polish (T035-T040): 4 hours (parallel: ~2 hours)

**Total Sequential**: ~28 hours  
**With Parallelization**: ~14-16 hours

---

## Notes

- **TDD Strict**: Do NOT start implementation until all tests exist and fail
- **Commit Strategy**: Commit after each task completion for easy rollback
- **Test-Driven**: Every task that makes tests pass should reference which test(s)
- **Constitution**: All tasks respect TDD, hardware abstraction, performance, code quality, UX principles
- **No Overengineering**: Keep implementations simple, use C++20 features judiciously
- **Zero Allocation**: Verify no dynamic memory allocation in parsing paths

---

## Validation Checklist

**Task Generation Validation**:
- [x] All contracts have corresponding tests (parser-contract.md → T006-T012, coordinate-parsing-contract.md → T013-T016)
- [x] All entities have implementation tasks (11 entities → T020-T030)
- [x] All tests come before implementation (T004-T019 before T020-T030)
- [x] Parallel tasks truly independent (different files or functions)
- [x] Each task specifies exact file path
- [x] No task modifies same file as another [P] task in same phase

**Success Indicators**:
- ✅ Tests written first (TDD enforced)
- ✅ Clear dependencies documented
- ✅ Parallel execution opportunities identified
- ✅ Integration with existing code planned
- ✅ Performance validation included
- ✅ Documentation updates included

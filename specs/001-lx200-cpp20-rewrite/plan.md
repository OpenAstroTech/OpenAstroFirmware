
# Implementation Plan: LX200 Protocol C++20 Rewrite

**Branch**: `001-lx200-cpp20-rewrite` | **Date**: 2025-10-04 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/home/andre/repos/OpenAstroTech/OpenAstroFirmware-workspace/OpenAstroFirmware/specs/001-lx200-cpp20-rewrite/spec.md`

## Execution Flow (/plan command scope)
```
1. Load feature spec from Input path
   → If not found: ERROR "No feature spec at {path}"
2. Fill Technical Context (scan for NEEDS CLARIFICATION)
   → Detect Project Type from file system structure or context (web=frontend+backend, mobile=app+api)
   → Set Structure Decision based on project type
3. Fill the Constitution Check section based on the content of the constitution document.
4. Evaluate Constitution Check section below
   → If violations exist: Document in Complexity Tracking
   → If no justification possible: ERROR "Simplify approach first"
   → Update Progress Tracking: Initial Constitution Check
5. Execute Phase 0 → research.md
   → If NEEDS CLARIFICATION remain: ERROR "Resolve unknowns"
6. Execute Phase 1 → contracts, data-model.md, quickstart.md, agent-specific template file (e.g., `CLAUDE.md` for Claude Code, `.github/copilot-instructions.md` for GitHub Copilot, `GEMINI.md` for Gemini CLI, `QWEN.md` for Qwen Code, or `AGENTS.md` for all other agents).
7. Re-evaluate Constitution Check section
   → If new violations: Refactor design, return to Phase 1
   → Update Progress Tracking: Post-Design Constitution Check
8. Plan Phase 2 → Describe task generation approach (DO NOT create tasks.md)
9. STOP - Ready for /tasks command
```

**IMPORTANT**: The /plan command STOPS at step 7. Phases 2-4 are executed by other commands:
- Phase 2: /tasks command creates tasks.md
- Phase 3-4: Implementation execution (manual or via tools)

## Summary

**Primary Requirement**: Rewrite the LX200 telescope protocol parser from C to modern C++20, maintaining protocol compatibility while improving code quality, testability, and maintainability.

**Technical Approach**: 
- Replace C implementation (`lib/lx200/lx200.c`) with C++20 implementation
- Use C++20 features (string_view, constexpr, concepts, std::optional) appropriately without overengineering
- Separate parsing logic from coordinate validation/conversion for independent testing
- Maintain zero dynamic allocation in critical paths
- Integrate with existing C++20 Mount control classes
- Use Zephyr logging framework for diagnostics
- Follow TDD: write comprehensive tests before implementation

## Technical Context

**Language/Version**: C++20 (application layer), Zephyr RTOS v4.2.0  
**Primary Dependencies**: 
- Zephyr RTOS v4.2.0 SDK
- Zephyr logging framework
- Zephyr Twister test framework
- C++ standard library (constrained for embedded)

**Storage**: N/A (protocol parser - no persistent storage)  
**Testing**: Zephyr Twister (unit and integration tests), native_sim for simulation testing  
**Target Platform**: 
- Primary: MKS Robin Nano (STM32F407-based), Nucleo F446RE
- Development: native_sim (x86_64 Linux)

**Project Type**: Embedded firmware (single project structure)  

**Performance Goals**: 
- Command response time: <100ms (REQ-PERF-002)
- Parser overhead: <10ms per command
- Zero dynamic memory allocation in parsing path
- Support concurrent command stream processing

**Constraints**: 
- Embedded memory constraints (RAM <60%, Flash <80%)
- No exceptions for error handling
- No RTTI (Run-Time Type Information)
- Real-time deterministic behavior required
- Must maintain LX200 protocol compatibility

**Scale/Scope**: 
- ~18 command families (A, B, C, D, F, G, g, H, I, L, M, P, Q, R, S, T, U, X)
- ~100+ individual LX200 commands
- 6 coordinate/time format types
- Support for high/low precision modes
- Integration with existing Mount, Stepper, Sensor subsystems

## Constitution Check
*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**I. Test-First Development:**
- [x] Test specifications written before implementation plan
  - Unit tests for command parsing, coordinate parsing, validation
  - Integration tests for complete command processing
  - Tests will be written first and must fail before implementation
- [x] Test scenarios will fail initially (no implementation yet)
  - Current C implementation will be deleted first
  - New C++20 implementation written test-first
- [x] All user stories have corresponding test cases
  - Command parsing scenarios → parser unit tests
  - Coordinate validation → coordinate parsing tests
  - Protocol compliance → integration tests with known command sequences
- [x] Native simulation tests planned before hardware tests
  - All tests run on native_sim platform first
  - Hardware testing only after simulation passes

**II. Hardware Abstraction & Modularity:**
- [x] Hardware dependencies abstracted through Zephyr device tree/drivers
  - LX200 parser is pure protocol logic with no hardware dependencies
  - Serial communication handled by Zephyr UART drivers (separate layer)
- [x] Core logic platform-independent (no board-specific code in application layer)
  - Parser implementation in C++20 application layer
  - No direct hardware access in parsing/validation code
- [x] Changes do not require modifications to unrelated subsystems
  - LX200 parser is self-contained module
  - Mount control calls parser through clean C++ interfaces
  - Stepper/sensor subsystems unaffected
- [x] Module boundaries clearly defined with public interfaces
  - Public header: `include/lx200/` (C++ interface)
  - Implementation: `lib/lx200/` (C++ source)
  - Tests: `tests/lib/lx200/`

**III. Real-Time Performance Requirements:**
- [x] Timing requirements identified (<100ms command response, sub-arcsecond tracking)
  - Command response: <100ms (REQ-PERF-002)
  - Parser overhead: <10ms budget per command
  - No blocking operations in parser
- [x] Performance impact assessed for control-path changes
  - Parser sits in command path but doesn't affect tracking loop
  - Command processing is asynchronous (REQ-COM-007)
  - No dynamic allocation eliminates allocation latency
- [x] No blocking operations in real-time control loops
  - Parser is called asynchronously from command handler
  - No waiting for I/O or long computations in parser
- [x] Resource constraints considered (memory, CPU usage)
  - Static buffer allocation (no dynamic memory)
  - Stack-based parsing (minimal heap usage)
  - Compile-time validation where possible (constexpr)

**IV. Code Quality & Standards:**
- [x] C++20 standards for application layer, C standards for drivers
  - Using C++20 features: string_view, constexpr, concepts, std::optional
  - Avoiding overengineering: no complex templates, deep inheritance
  - RAII for resource management
- [x] Error handling at all external boundaries
  - std::optional/result types for error conditions (no exceptions)
  - Validation at parsing boundaries
  - Clear error reporting for invalid commands
- [x] Logging using Zephyr framework with appropriate levels
  - LOG_MODULE_REGISTER for module-level logging
  - DEBUG for parsing details, INFO for commands, ERR for failures
- [x] Static analysis and linting planned in CI/CD
  - Compiler warnings enabled (-Wall -Wextra)
  - Zephyr build system static analysis
  - clang-tidy for C++ linting (future)

**V. User Experience Consistency:**
- [x] LX200 protocol compliance verified for command changes
  - Strict adherence to Meade LX200 protocol specification
  - Response formats match expected patterns (HH:MM:SS#, etc.)
  - All command families from specification supported
- [x] Third-party software compatibility considered (ASCOM, INDI, etc.)
  - Protocol behavior unchanged (only implementation language changed)
  - Command responses identical to C version
  - Integration tests verify protocol compliance
- [x] Documentation updates planned for user-facing changes
  - No user-facing changes (internal rewrite)
  - Developer documentation for C++ API
  - Migration guide for contributors
- [x] Configuration/calibration data preservation addressed (if firmware update related)
  - N/A - parser has no persistent configuration
  - Precision mode setting preserved in parser state

## Project Structure

### Documentation (this feature)
```
specs/[###-feature]/
├── plan.md              # This file (/plan command output)
├── research.md          # Phase 0 output (/plan command)
├── data-model.md        # Phase 1 output (/plan command)
├── quickstart.md        # Phase 1 output (/plan command)
├── contracts/           # Phase 1 output (/plan command)
└── tasks.md             # Phase 2 output (/tasks command - NOT created by /plan)
```

### Source Code (repository root)

```
OpenAstroFirmware/
├── include/
│   └── lx200/
│       └── lx200.hpp           # NEW: Public C++20 interface (replaces lx200.h)
│
├── lib/
│   └── lx200/
│       ├── CMakeLists.txt      # Updated: build C++ instead of C
│       ├── Kconfig             # Unchanged: LX200 configuration
│       ├── lx200.cpp           # NEW: C++20 implementation (replaces lx200.c)
│       ├── parser.cpp          # NEW: Command parsing logic
│       ├── coordinates.cpp     # NEW: Coordinate parsing/validation
│       ├── parser.hpp          # NEW: Internal parser interface
│       └── coordinates.hpp     # NEW: Internal coordinate interface
│
├── tests/
│   └── lib/
│       └── lx200/
│           ├── CMakeLists.txt        # Updated: test C++ implementation
│           ├── src/
│           │   ├── main.cpp          # NEW: Test suite entry (C++)
│           │   ├── test_parser.cpp   # NEW: Command parser tests
│           │   ├── test_coordinates.cpp  # NEW: Coordinate parsing tests
│           │   ├── test_commands.cpp # NEW: Command identification tests
│           │   └── test_integration.cpp  # NEW: End-to-end protocol tests
│           └── testcase.yaml         # Updated: test configurations
│
└── app/
    └── src/
        └── mount/
            └── Mount.cpp       # Modified: calls new C++ LX200 interface
```

**Files to DELETE:**
- `lib/lx200/lx200.c` - Current C implementation
- `include/lx200/lx200.h` - Current C header
- `tests/lib/lx200/src/main.c` - Current C test suite

**Structure Decision**: 
Embedded firmware single-project structure using Zephyr module conventions. LX200 library is a self-contained module with:
- Public C++20 interface in `include/lx200/`
- Implementation in `lib/lx200/` using C++20
- Comprehensive test suite in `tests/lib/lx200/`
- Integration with existing Mount controller through C++ interfaces
- Build system uses Zephyr CMake + west workspace management

## Phase 0: Outline & Research
1. **Extract unknowns from Technical Context** above:
   - For each NEEDS CLARIFICATION → research task
   - For each dependency → best practices task
   - For each integration → patterns task

2. **Generate and dispatch research agents**:
   ```
   For each unknown in Technical Context:
     Task: "Research {unknown} for {feature context}"
   For each technology choice:
     Task: "Find best practices for {tech} in {domain}"
   ```

3. **Consolidate findings** in `research.md` using format:
   - Decision: [what was chosen]
   - Rationale: [why chosen]
   - Alternatives considered: [what else evaluated]

**Output**: research.md with all NEEDS CLARIFICATION resolved

## Phase 1: Design & Contracts
*Prerequisites: research.md complete*

1. **Extract entities from feature spec** → `data-model.md`:
   - Entity name, fields, relationships
   - Validation rules from requirements
   - State transitions if applicable

2. **Generate API contracts** from functional requirements:
   - For each user action → endpoint
   - Use standard REST/GraphQL patterns
   - Output OpenAPI/GraphQL schema to `/contracts/`

3. **Generate contract tests** from contracts:
   - One test file per endpoint
   - Assert request/response schemas
   - Tests must fail (no implementation yet)

4. **Extract test scenarios** from user stories:
   - Each story → integration test scenario
   - Quickstart test = story validation steps

5. **Update agent file incrementally** (O(1) operation):
   - Run `.specify/scripts/bash/update-agent-context.sh copilot`
     **IMPORTANT**: Execute it exactly as specified above. Do not add or remove any arguments.
   - If exists: Add only NEW tech from current plan
   - Preserve manual additions between markers
   - Update recent changes (keep last 3)
   - Keep under 150 lines for token efficiency
   - Output to repository root

**Output**: data-model.md, /contracts/*, failing tests, quickstart.md, agent-specific file

## Phase 2: Task Planning Approach
*This section describes what the /tasks command will do - DO NOT execute during /plan*

**Task Generation Strategy**:
- Load `.specify/templates/tasks-template.md` as base
- Generate tasks from Phase 1 design docs (contracts, data model, quickstart)
- Each contract → contract test task [P]
- Each entity → model creation task [P] 
- Each user story → integration test task
- Implementation tasks to make tests pass

**Ordering Strategy**:
- TDD order: Tests before implementation 
- Dependency order: Models before services before UI
- Mark [P] for parallel execution (independent files)

**Estimated Output**: 25-30 numbered, ordered tasks in tasks.md

**IMPORTANT**: This phase is executed by the /tasks command, NOT by /plan

## Phase 3+: Future Implementation
*These phases are beyond the scope of the /plan command*

**Phase 3**: Task execution (/tasks command creates tasks.md)  
**Phase 4**: Implementation (execute tasks.md following constitutional principles)  
**Phase 5**: Validation (run tests, execute quickstart.md, performance validation)

## Complexity Tracking
*Fill ONLY if Constitution Check has violations that must be justified*

**No violations** - All constitutional requirements satisfied:
- TDD enforced through test-first approach
- Hardware abstraction maintained (pure protocol logic)
- Performance requirements met (zero allocation, <10ms parsing)
- C++20 standards followed without overengineering
- LX200 protocol compliance ensures UX consistency

## Progress Tracking
*This checklist is updated during execution flow*

**Phase Status**:
- [x] Phase 0: Research complete (/plan command) - research.md created
- [x] Phase 1: Design complete (/plan command) - data-model.md, contracts/, quickstart.md created
- [x] Phase 2: Task planning complete (/plan command - approach described below)
- [ ] Phase 3: Tasks generated (/tasks command) - NOT executed by /plan
- [ ] Phase 4: Implementation complete
- [ ] Phase 5: Validation passed

**Gate Status**:
- [x] Initial Constitution Check: PASS (all requirements met)
- [x] Post-Design Constitution Check: PASS (no violations)
- [x] All NEEDS CLARIFICATION resolved (via spec.md clarifications section)
- [x] Complexity deviations documented (none)

---
*Based on Constitution v2.1.1 - See `/memory/constitution.md`*

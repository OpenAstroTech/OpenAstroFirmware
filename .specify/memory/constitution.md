<!--
SYNC IMPACT REPORT
==================
Version Change: 1.0.3 → 1.1.0
Amendment Type: Material Addition (MINOR version bump)
Modified Sections:
  - Principle II: Code Quality & Maintainability
    * Added #2: Minimal Complexity principle (YAGNI enforcement)
    * Modified #8: Modern C++ Features - added qualifier "when they reduce complexity or improve safety"
    * Updated rationale to emphasize "simplicity first" and "every layer must earn its place"
  - Code Review Gates
    * Added explicit complexity justification requirement
  - Compliance Verification
    * Added complexity minimization as explicit verification requirement
    * Added requirement for reviewers to challenge unjustified abstractions
  - Governance
    * Removed GitHub discussions requirement from Amendment Process
    * Updated Continuous Improvement to include complexity justification requirement
    * Removed GitHub discussions from community feedback collection

Rationale for Version 1.1.0:
  - MINOR bump: Material expansion of governance with new principle
  - "Minimal Complexity" is a substantive addition to code quality standards
  - Elevates complexity minimization to constitutional principle (was implicit, now explicit)
  - Changes review process to actively challenge complexity
  - Removes platform-specific requirement (GitHub discussions)
  - No breaking changes but materially expands principle scope

Previous Version History:
  v1.0.3 (2025-10-12): Clarified LX200 error handling via logging
  v1.0.2 (2025-10-12): Added explicit .clang-format compliance requirement
  v1.0.1 (2025-10-12): Clarified lib/ uses C++20, drivers/ uses C
  v1.0.0 (2025-10-12): Initial constitution establishing project governance

Templates Status:
  ✅ plan-template.md - Constitution Check section aligns with principles
  ✅ spec-template.md - Requirements structure supports testable acceptance criteria
  ✅ tasks-template.md - Task organization supports test-first workflow
  ⚠️  No commands directory found - no agent-specific references to update

Follow-up TODOs:
  - Review existing codebase for unnecessary complexity
  - Document complexity justification template for reviewers
  - Add "complexity impact" section to PR template
  - Create examples of justified vs unjustified abstractions
  - Audit current C++20 feature usage for complexity/benefit ratio
  - Monitor compliance as features are implemented
-->

# OpenAstroFirmware Constitution

## Core Principles

### I. Test-First Development (NON-NEGOTIABLE)

All implementations MUST follow the Test-Driven Development (TDD) cycle:

1. **Write Tests First**: Tests define specifications before implementation begins
2. **Red-Green-Refactor**: Tests MUST fail initially, then pass after implementation, then be refactored
3. **Zephyr Twister Integration**: All tests MUST be runnable via `west twister` on both hardware and native_sim
4. **Multi-Platform Validation**: Tests MUST pass on target hardware (robin_nano) AND native simulation
5. **Stub Documentation**: Unimplemented functions MUST be documented as stubs with corresponding failing tests that specify expected behavior

**Rationale**: Embedded firmware requires high reliability. Test-first development ensures correctness before deployment to hardware where debugging is costly. The stub-with-test pattern maintains architectural clarity during incremental development.

**Test Coverage Requirements**:
- Contract tests for all protocol implementations (LX200 commands)
- Integration tests for stepper motor control sequences
- Unit tests for coordinate parsing and astronomical calculations
- Hardware abstraction layer tests for device tree configurations

### II. Code Quality & Maintainability

Code MUST adhere to strict quality standards enabling long-term maintainability:

1. **Language Separation**: C++20 for application layer (`app/src/`) and reusable libraries (`lib/`), pure C for low-level drivers (`drivers/`) requiring hardware abstraction portability
2. **Minimal Complexity**: Complexity MUST be minimized; every abstraction, pattern, or feature MUST be justified by concrete requirements; YAGNI (You Aren't Gonna Need It) principle enforced
3. **Consistent Formatting**: All code MUST conform to `.clang-format` specification; automated formatting required before commit
4. **Zephyr Module Structure**: Follow `zephyr/module.yml` conventions with proper board_root, dts_root, and CMakeLists integration
5. **Structured Logging**: Use Zephyr LOG_MODULE_REGISTER with appropriate log levels (DBG, INF, WRN, ERR)
6. **Device Tree First**: Hardware configuration via device tree overlays, NOT compile-time #defines
7. **No Magic Numbers**: All constants named and documented; astronomical precision values explicitly justified
8. **Modern C++ Features**: Leverage C++20 features (concepts, ranges, coroutines) for application logic and library implementation clarity when they reduce complexity or improve safety

**Rationale**: The v1 OpenAstroTracker firmware suffered from maintainability issues due to accumulated complexity. This rewrite prioritizes simplicity first: clean architecture, clear separation of concerns, and leveraging modern language features only when they demonstrably reduce complexity or improve safety. Every layer of abstraction must earn its place. C++20 enables safer abstractions in libraries while pure C drivers maintain maximum portability across hardware platforms.

**Code Review Gates**:
- All PRs MUST pass automated formatting checks against `.clang-format` (run `clang-format` before commit)
- All PRs MUST pass linting and static analysis checks
- Complexity additions (new abstractions, patterns, dependencies) MUST be justified with concrete use cases
- Device tree changes MUST include board-specific overlays
- Libraries (`lib/`) MUST use C++20 with proper namespaces and encapsulation
- Low-level drivers (`drivers/`) MUST maintain C compatibility for maximum portability
- Log statements MUST use structured format with module registration

### III. Real-Time Performance & Reliability

System MUST meet deterministic performance requirements for telescope tracking:

1. **Tracking Precision**: Stepper control loop MUST maintain ±5 arcsecond precision
2. **Command Latency**: LX200 protocol responses MUST complete within 100ms for synchronous commands
3. **Memory Constraints**: Application MUST operate within 128KB RAM budget on STM32F407
4. **Deterministic Timing**: Critical tracking operations MUST use Zephyr RTOS threads with priority scheduling
5. **Fail-Safe Mechanisms**: Motor control MUST implement timeout-based safety stops
6. **No Blocking Operations**: Serial I/O and protocol parsing MUST be asynchronous/non-blocking

**Rationale**: Real-time telescope tracking requires predictable timing. Missed deadlines cause tracking errors visible in astrophotography. Resource constraints are hardware limitations that cannot be violated.

**Performance Testing Requirements**:
- Latency measurements for all command paths
- Memory profiling during typical operation sequences
- Timing analysis for stepper interrupt service routines
- Load testing with multiple concurrent LX200 command streams

### IV. User Experience Consistency

User-facing behavior MUST be predictable and compatible across platforms:

1. **LX200 Protocol Compliance**: Full compatibility with ASCOM, INDI, and Stellarium clients
2. **Cross-Platform Parity**: Features MUST behave identically on all supported hardware (robin_nano, nucleo_f446re)
3. **Coordinate Format Standards**: Strict adherence to `HH:MM:SS` (RA) and `sDD*MM:SS` (Dec) formats
4. **Error Communication**: LX200 protocol limitations prevent in-band error messages; all errors, warnings, and diagnostics MUST be logged via Zephyr logging system for troubleshooting
5. **Configuration Portability**: Device tree overlays allow board-specific tuning without recompilation
6. **Simulation Fidelity**: native_sim MUST accurately represent hardware behavior for development

**Rationale**: Users expect industry-standard LX200 behavior. Inconsistencies between hardware platforms or simulation/reality mismatches erode trust. Configuration via device tree enables community hardware variations without firmware forks. The LX200 protocol's limited error signaling requires comprehensive logging for debugging and system monitoring.

**UX Validation Gates**:
- LX200 command compatibility verified against reference telescope implementations
- Cross-board testing for all features before release
- Simulation tests MUST correlate with hardware test results
- All error conditions MUST have corresponding log messages (ERR or WRN level)
- User-reported incompatibilities prioritized as critical bugs

## Quality Gates & Standards

### Build System Requirements

- **West Workspace**: All builds MUST use `west build` with proper workspace initialization
- **Configuration Overlays**: Production (`prj.conf`) and debug (`debug.conf`) configurations maintained
- **Multi-Board Support**: Single codebase builds for robin_nano, nucleo_f446re, and native_sim without modification
- **Clean Builds**: `west build -t pristine` MUST complete without errors before PR merge

### Documentation Standards

- **Architectural Decision Records**: Major design choices documented in `specs/design.md`
- **Requirements Traceability**: All features linked to requirements in `specs/requirements.md`
- **Protocol References**: LX200 implementation documented against `lx200-protocol-llm-reference.md`
- **Inline Documentation**: All public APIs documented with Doxygen-compatible comments

### Development Workflow

- **Feature Branches**: All work on branches named `###-feature-name` with issue numbers
- **Specification First**: Features require approved spec in `specs/###-feature-name/spec.md` before implementation
- **Incremental Delivery**: User stories prioritized (P1, P2, P3) for independent testing and delivery
- **CI/CD Integration**: Twister test suite runs automatically on all PRs

## Governance

This constitution establishes the non-negotiable principles for OpenAstroFirmware development. All contributors, maintainers, and code reviewers MUST enforce these principles.

**Amendment Process**:
1. Proposed amendments MUST be documented with rationale and impact analysis
2. Amendments require maintainer consensus
3. Version bumping follows semantic versioning:
   - **MAJOR**: Backward-incompatible principle changes or removals
   - **MINOR**: New principles added or material expansions
   - **PATCH**: Clarifications, wording improvements, non-semantic refinements
4. Amendment adoption requires update of dependent templates and specification files

**Compliance Verification**:
- All PRs MUST pass constitution compliance checks (automated where possible)
- Code reviews MUST explicitly verify test-first workflow was followed
- Performance requirements verified via automated test suite
- Complexity MUST be minimized; any added complexity MUST be justified with concrete requirements and architectural rationale
- Reviewers MUST challenge abstractions that don't demonstrably simplify the codebase

**Constitutional Authority**:
- This constitution supersedes conflicting guidance in README, contributing guides, or individual specifications
- When ambiguity arises, constitution principles take precedence
- Runtime development guidance uses `.github/copilot-instructions.md` for AI agent context

**Continuous Improvement**:
- Constitution reviewed quarterly for alignment with project evolution
- Amendments proposed based on lessons learned from feature implementations
- Complexity introductions require explicit justification

**Version**: 1.1.0 | **Ratified**: 2025-10-12 | **Last Amended**: 2025-10-12
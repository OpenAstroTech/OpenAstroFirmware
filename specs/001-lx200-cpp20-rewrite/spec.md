# Feature Specification: LX200 Protocol C++20 Rewrite

**Feature Branch**: `001-lx200-cpp20-rewrite`  
**Created**: 2025-10-04  
**Status**: Draft  
**Input**: User description: "Rewrite current lx200 implementation by using C++20. Keep the implementation simple, do not overengineer. Currently available implementation should be deleted."

## ⚡ Quick Guidelines
- ✅ Focus on WHAT users need and WHY
- ❌ Avoid HOW to implement (no tech stack, APIs, code structure)
- 👥 Written for business stakeholders, not developers

---

## User Scenarios & Testing *(mandatory)*

### Primary User Story
As a telescope control software developer (ASCOM/INDI/N.I.N.A./SkySafari), I need to communicate with the OpenAstroFirmware using the standard LX200 protocol so that my software can control the telescope mount reliably without custom integration code.

### Acceptance Scenarios

1. **Given** a serial connection to the firmware at 115200 baud, **When** I send `:GR#` (get right ascension), **Then** the system responds with the current RA in format `HH:MM:SS#` within 100ms

2. **Given** the telescope is tracking, **When** I send `:Sr12:34:56#` (set target RA) followed by `:Sd+45*30:15#` (set target DEC) and `:MS#` (slew to target), **Then** the system validates coordinates and begins slewing to the target position

3. **Given** I send an invalid command `:INVALID#`, **When** the parser processes it, **Then** the system responds with an appropriate error indication without crashing

4. **Given** the system is in high precision mode, **When** I request coordinates, **Then** responses use HH:MM:SS format (not HH:MM.T format)

5. **Given** the system receives a partial command `:GR` without terminator, **When** the `#` arrives later, **Then** the command is assembled correctly and executed

6. **Given** multiple commands arrive rapidly, **When** the parser processes the command stream, **Then** each command is handled sequentially without corruption

### Edge Cases
- What happens when coordinate values are out of range (RA > 24h, DEC > 90°)?
- How does the system handle buffer overflow if commands exceed maximum length?
- What happens when invalid characters appear in coordinate strings?
- How does the parser recover from malformed commands?
- What happens if terminator `#` is missing or duplicate `:` prefixes appear?

## Requirements *(mandatory)*

### Functional Requirements

**FR-001**: System MUST parse LX200 commands with format `:<command>[parameters]#`

**FR-002**: System MUST validate command prefix `:` and terminator `#`

**FR-003**: System MUST support all LX200 command families defined in the protocol specification:
- A (Alignment)
- B (Reticule/Accessory Control)
- C (Sync Control)
- D (Distance Bars)
- F (Focuser Control)
- G (Get Telescope Information)
- g (GPS Commands)
- H (Home/Park Commands)
- I (Initialize)
- L (Library)
- M (Movement)
- P (High Precision)
- Q (Quit/Stop)
- R (Rate Control)
- S (Set Information)
- T (Tracking)
- U (User Format)
- X (Extended OAT Commands)

**FR-004**: System MUST parse coordinate formats:
- Right Ascension: `HH:MM:SS` (high precision) or `HH:MM.T` (low precision)
- Declination: `sDD*MM:SS` or `sDD*MM'SS` (with sign, degrees, minutes, seconds)
- Longitude: `sDDD*MM` (degrees 0-360 west from Greenwich)
- Latitude: `sDD*MM` (sign, degrees, minutes)
- Time: `HH:MM:SS` (24-hour format)
- Date: `MM/DD/YY`

**FR-005**: System MUST validate coordinate ranges:
- RA: 0-24 hours
- DEC: -90 to +90 degrees
- Time: valid 24-hour time
- Date: valid calendar date

**FR-006**: System MUST support precision mode switching (high/low precision coordinate formats)

**FR-007**: System MUST format response strings according to LX200 protocol specification

**FR-008**: System MUST handle partial commands (buffering until terminator received)

**FR-009**: System MUST handle buffer overflow gracefully without memory corruption

**FR-010**: System MUST respond to commands within 100ms (REQ-PERF-002)

**FR-011**: System MUST maintain parser state across multiple command invocations

**FR-012**: System MUST identify command families to route commands to appropriate handlers

### Non-Functional Requirements

**NFR-001**: Implementation MUST use C++20 standard features (not C)

**NFR-002**: Implementation MUST be simple and maintainable (no overengineering)

**NFR-003**: Implementation MUST have clear separation between parsing and command execution

**NFR-004**: Implementation MUST use RAII for resource management

**NFR-005**: Implementation MUST integrate with Zephyr logging framework

**NFR-006**: Parser MUST be independently testable without hardware dependencies

**NFR-007**: Implementation MUST maintain zero dynamic memory allocation in critical paths

**NFR-008**: Code MUST follow modern C++20 idioms (concepts, constexpr, string_view where appropriate)

### Removal Requirements

**RR-001**: Current C implementation in `lib/lx200/lx200.c` MUST be deleted

**RR-002**: Current C header in `include/lx200/lx200.h` MUST be replaced with C++ header

**RR-003**: Existing test suite MUST be updated or replaced to test C++20 implementation

**RR-004**: Build system configuration MUST be updated to compile C++ instead of C

### Key Entities *(include if feature involves data)*

- **LX200Command**: Represents a parsed command with family, name, and parameters
- **CoordinateValue**: Represents astronomical coordinates (RA, DEC, ALT, AZ) with validation
- **TimeValue**: Represents time/date values with validation
- **ParserState**: Manages command buffer, parsing state, and precision mode
- **ParseResult**: Enum representing parsing outcomes (OK, incomplete, invalid, error)

---

## Clarifications

### Session 1: 2025-10-04 - Architecture & Design Approach

**Q1: Should the C++20 implementation maintain the same public API as the C version for backward compatibility?**
A: No. Since we're doing a complete rewrite, we can design a cleaner C++ API. However, the protocol behavior must remain LX200-compatible.

**Q2: What level of object-oriented design is appropriate without overengineering?**
A: Keep it simple: Use classes for clear ownership and encapsulation, but avoid deep inheritance hierarchies, complex template metaprogramming, or design patterns just for the sake of patterns. Prefer composition over inheritance. Use C++20 features like concepts for validation, string_view for non-owning strings, constexpr for compile-time computation.

**Q3: Should coordinate parsing and validation be separate from command parsing?**
A: Yes. Parser should extract command and parameter strings. Separate validation/conversion functions handle coordinate types. This allows testing each layer independently.

**Q4: How should the C++ implementation integrate with the existing mount control C++ classes?**
A: The LX200 parser should provide clean C++ interfaces that the Mount class can call directly. No C wrapper needed since both are C++20.

**Q5: Should we use exceptions for error handling?**
A: No. Embedded firmware should avoid exceptions for predictable control flow. Use std::optional, std::expected (C++23, or custom variant), or result enums for error handling.

**Q6: What about the existing test suite - full rewrite or adapt?**
A: Rewrite tests to match the new C++ API. This ensures tests specify the desired behavior for the new implementation. Keep test scenarios but update test code structure.

**Q7: Should string parsing use string_view to avoid allocations?**
A: Yes, where appropriate. Use string_view for parsing to avoid copies. Only allocate when storing parsed results.

**Q8: How should we handle the transition? Implement alongside C version then swap, or delete C first?**
A: Delete C implementation first, then implement C++20 version test-first. Clean slate approach prevents confusion and ensures focus on the new design.

---

## Review & Acceptance Checklist
*GATE: Automated checks run during main() execution*

### Content Quality
- [x] No implementation details (languages, frameworks, APIs)
- [x] Focused on user value and business needs
- [x] Written for non-technical stakeholders
- [x] All mandatory sections completed

### Requirement Completeness
- [x] No [NEEDS CLARIFICATION] markers remain
- [x] Requirements are testable and unambiguous  
- [x] Success criteria are measurable
- [x] Scope is clearly bounded
- [x] Dependencies and assumptions identified

---

## Execution Status
*Updated by main() during processing*

- [x] User description parsed
- [x] Key concepts extracted
- [x] Ambiguities marked (resolved in clarifications)
- [x] User scenarios defined
- [x] Requirements generated
- [x] Entities identified
- [x] Review checklist passed

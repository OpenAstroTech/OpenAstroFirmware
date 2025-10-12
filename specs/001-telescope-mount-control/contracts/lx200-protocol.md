# LX200 Protocol Contracts

**Feature**: Telescope Mount Control System  
**Phase**: 1 - Design & Contracts  
**Date**: 2025-10-12

## Overview

This document defines the complete contract for LX200 protocol implementation. All commands follow the format `:COMMAND[parameters]#` where `:` is the start marker, `COMMAND` is 1-2 characters, optional parameters follow, and `#` is the terminator.

## Protocol Fundamentals

### Message Format

**Request**: `:COMMAND[params]#`
- Start: `:` (ASCII 0x3A)
- Command: 1-2 alphanumeric characters
- Parameters: Optional, format varies by command
- Terminator: `#` (ASCII 0x23)

**Response Types**:
1. **Acknowledge**: `1` - Command accepted and executed
2. **Reject**: `0` - Command rejected or failed
3. **String**: Variable text (coordinates, status, etc.)
4. **None**: No response (command processed silently)

### Coordinate Formats

**Right Ascension (RA)**:
- High Precision: `HH:MM:SS` (hours:minutes:seconds)
- Low Precision: `HH:MM.T` (hours:minutes.tenths)
- Range: 00:00:00 to 23:59:59

**Declination (Dec)**:
- Format: `sDD*MM:SS` or `sDD*MM'SS`
  - `s` = sign (`+` for north, `-` for south)
  - `DD` = degrees (00-90)
  - `MM` = arcminutes (00-59)
  - `SS` = arcseconds (00-59)
  - `*` or `'` = separator (degree symbol or apostrophe)

**Examples**:
```
RA:  "12:34:56" = 12h 34m 56s
Dec: "+45*12:34" = +45° 12' 34"
Dec: "-12*00:00" = -12° 00' 00"
```

## Command Categories

### 0. Initialization Commands

#### I - Initialize Telescope
**Request**: `:I#`
**Response**: None
**Effects**: Initializes mount, enters Serial Control Mode, displays RA/DEC on LCD
**Notes**: Should be first command sent after connection, exits remote control mode

### 1. Alignment & Sync Commands

#### CM - Synchronize Database (Sync)
**Request**: `:CM#`
**Response**: `"NONE#"` (standard LX200) or empty
**Preconditions**: Target coordinates set via `:Sr#` and `:Sd#`
**Effects**: Synchronizes mount to current target coordinates
**Notes**: Tells mount what coordinates it is currently pointing at

### 2. Get Commands (Query Mount State)

#### GR - Get Right Ascension
**Request**: `:GR#`
**Response**: `"HH:MM:SS#"` (high precision) or `"HH:MM.T#"` (low precision)
**Example**: `"12:34:56#"` = mount currently at RA 12h 34m 56s
**Errors**: None (always returns current position)

#### GD - Get Declination
**Request**: `:GD#`
**Response**: `"sDD*MM:SS#"`
**Example**: `"+45*12:34#"` = mount currently at +45° 12' 34"
**Errors**: None (always returns current position)

#### Gt - Get Site Latitude
**Request**: `:Gt#`
**Response**: `"sDD*MM#"`
**Example**: `"+35*30#"` = 35.5° north latitude
**Errors**: Returns `"+00*00#"` if location not set

#### Gg - Get Site Longitude
**Request**: `:Gg#`
**Response**: `"sDDD*MM#"` (note: 3 digits for degrees)
**Example**: `"-122*24#"` = 122.4° west longitude
**Errors**: Returns `"+000*00#"` if location not set

#### GL - Get Local Time (12-hour)
**Request**: `:GL#`
**Response**: `"HH:MM:SS#"`
**Example**: `"09:30:45#"` = 9:30:45 AM
**Notes**: Requires system time set via `:SL#`

#### GC - Get Calendar Date
**Request**: `:GC#`
**Response**: `"MM/DD/YY#"`
**Example**: `"10/12/25#"` = October 12, 2025
**Notes**: Requires date set via `:SC#`

### 3. Set Commands (Configure Mount)

#### Sr - Set Target Right Ascension
**Request**: `:SrHH:MM:SS#` or `:SrHH:MM.T#`
**Response**: `1` (valid) or `0` (invalid)
**Example**: `:Sr12:34:56#` → `1`
**Validation**: 
- Hours: 0-23
- Minutes: 0-59
- Seconds: 0-59.9
**Effects**: Sets target RA for next slew command

#### Sd - Set Target Declination
**Request**: `:SdsDD*MM:SS#` or `:SdsDD*MM#`
**Response**: `1` (valid) or `0` (invalid)
**Example**: `:Sd+45*12:34#` → `1`
**Validation**:
- Sign: `+` or `-`
- Degrees: 0-90
- Arcminutes: 0-59
- Arcseconds: 0-59
**Effects**: Sets target Dec for next slew command

#### St - Set Site Latitude
**Request**: `:StsDD*MM#`
**Response**: `1` (valid) or `0` (invalid)
**Example**: `:St+35*30#` → `1`
**Validation**: -90° to +90°
**Effects**: Updates observer location for celestial calculations

#### Sg - Set Site Longitude
**Request**: `:SgsDDD*MM#`
**Response**: `1` (valid) or `0` (invalid)
**Example**: `:Sg-122*24#` → `1`
**Validation**: -180° to +180° (or 0-360° west from Greenwich)
**Effects**: Updates observer location for celestial calculations

#### SL - Set Local Time
**Request**: `:SLHH:MM:SS#`
**Response**: `1` (valid) or `0` (invalid)
**Example**: `:SL09:30:45#` → `1`
**Validation**: HH (0-23), MM (0-59), SS (0-59)
**Effects**: Sets system time for tracking calculations

#### SC - Set Calendar Date
**Request**: `:SCMM/DD/YY#`
**Response**: `"Updating        Planetary Data#"` then `1`
**Example**: `:SC10/12/25#` → (message) → `1`
**Validation**: Valid calendar date
**Effects**: Sets system date for ephemeris calculations

### 4. Motion Commands

#### MS - Slew to Target
**Request**: `:MS#`
**Response**: `0` (slew started) or `1` (error), then `"Object Below Horizon#"` if error
**Preconditions**: Target RA/Dec set via `:Sr#` and `:Sd#`
**Effects**: Begins slewing mount to target coordinates
**Errors**: 
- No target set: `1`, `"Object Below Horizon#"`
- Target unreachable: `1`, `"Object Below Horizon#"`
- Already slewing: `0` (continues current slew)

#### Me, Mw, Mn, Ms - Slew Direction
**Requests**: 
- `:Me#` - East
- `:Mw#` - West
- `:Mn#` - North
- `:Ms#` - South
**Response**: None
**Effects**: Starts slewing in specified direction at current rate
**Notes**: Continues until `:Qe#`, `:Qw#`, `:Qn#`, or `:Qs#` received

#### Qe, Qw, Qn, Qs - Halt Slew Direction
**Requests**:
- `:Qe#` - Halt east
- `:Qw#` - Halt west
- `:Qn#` - Halt north
- `:Qs#` - Halt south
**Response**: None
**Effects**: Stops slewing in specified direction

#### Q - Halt All Motion
**Request**: `:Q#`
**Response**: None
**Effects**: Emergency stop - halts all motors immediately
**Priority**: Highest (preempts all other commands)

### 5. Tracking Commands

#### T+ - Enable Sidereal Tracking
**Request**: `:T+#`
**Response**: None
**Effects**: Enables sidereal tracking at 15.041067 arcsec/sec
**Notes**: Tracking continues until `:T-#` or `:Q#`

#### T- - Disable Tracking
**Request**: `:T-#`
**Response**: None
**Effects**: Disables tracking, mount holds position

#### TL - Set Lunar Tracking Rate
**Request**: `:TL#`
**Response**: None
**Effects**: Sets tracking rate to lunar (14.515 arcsec/sec)

#### TS - Set Solar Tracking Rate
**Request**: `:TS#`
**Response**: None
**Effects**: Sets tracking rate to solar (15.0 arcsec/sec)

#### TQ - Set Sidereal Tracking Rate
**Request**: `:TQ#`
**Response**: None
**Effects**: Sets tracking rate to sidereal (15.041067 arcsec/sec) - default

### 6. Slew Rate Commands

#### RC - Set Slew Rate to Centering
**Request**: `:RC#`
**Response**: None
**Effects**: Sets slew rate for manual centering (medium speed)

#### RG - Set Slew Rate to Guiding
**Request**: `:RG#`
**Response**: None
**Effects**: Sets slew rate for autoguider corrections (slowest)

#### RM - Set Slew Rate to Find
**Request**: `:RM#`
**Response**: None
**Effects**: Sets slew rate for finding objects (medium-fast)

#### RS - Set Slew Rate to Slew
**Request**: `:RS#`
**Response**: None
**Effects**: Sets slew rate to maximum safe speed (fastest)

### 7. Focus Control Commands

#### F+ - Start Focuser Motion In
**Request**: `:F+#`
**Response**: None
**Effects**: Starts focuser motor moving inward
**Notes**: Out of scope for initial implementation (mount control only)

#### F- - Start Focuser Motion Out
**Request**: `:F-#`
**Response**: None
**Effects**: Starts focuser motor moving outward
**Notes**: Out of scope for initial implementation

#### FQ - Halt Focuser Motion
**Request**: `:FQ#`
**Response**: None
**Effects**: Stops focuser motor
**Notes**: Out of scope for initial implementation

### 8. Distance/Status Commands

#### D - Query Slew Status
**Request**: `:D#`
**Response**: `"|#"` (still slewing) or `" #"` (slewing complete)
**Notes**: Polled by client to detect slew completion
**Example**:
```
:MS#  → 0 (start slew)
:D#   → "|#" (still slewing)
:D#   → "|#" (still slewing)
:D#   → " #" (complete - note the space)
```

### 9. Precision Commands

#### P - High Precision Mode
**Request**: `:P#`
**Response**: None
**Effects**: Future coordinate responses use high precision format (HH:MM:SS)
**Default**: Low precision (HH:MM.T) unless `:P#` sent

#### U - Toggle Precision
**Request**: `:U#`
**Response**: None
**Effects**: Toggles between high and low precision modes

### 10. Additional Get Commands

#### Ga - Get Local Time (12-hour format)
**Request**: `:Ga#`
**Response**: `"HH:MM:SS#"`
**Notes**: Returns time in 12-hour format (modulo 12)

#### Gc - Get Clock Format
**Request**: `:Gc#`
**Response**: `"24#"`
**Notes**: Always returns 24-hour format

#### GG - Get UTC Offset
**Request**: `:GG#`
**Response**: `"sHH#"`
**Example**: `"-08#"` for Pacific Time
**Notes**: Returns offset from UTC in hours (negative of timezone offset)

#### GT - Get Tracking Rate
**Request**: `:GT#`
**Response**: `"60.0#"`
**Notes**: Returns tracking rate in some unit

#### GM, GN, GO, GP - Get Site Names
**Request**: `:GM#`, `:GN#`, `:GO#`, `:GP#`
**Response**: `"OAT1#"`, `"OAT2#"`, `"OAT3#"`, `"OAT4#"`
**Notes**: Site name storage (4 slots)

#### Gd - Get Target Declination
**Request**: `:Gd#`
**Response**: `"sDD*MM'SS#"`
**Notes**: Returns target (not current) declination

#### Gr - Get Target Right Ascension
**Request**: `:Gr#`
**Response**: `"HH:MM:SS#"`
**Notes**: Returns target (not current) RA

### 11. Additional Set Commands

#### SHP - Set Home Point
**Request**: `:SHP#`
**Response**: `1`
**Effects**: Sets current orientation as home position

#### SH - Set Hour Angle
**Request**: `:SHHH:MM#`
**Response**: `1` (success) or `0` (failure)
**Effects**: Sets Hour Angle (should be Polaris HA)

#### SHL - Set LST Time
**Request**: `:SHLHH:MM#`
**Response**: `1` (success) or `0` (failure)
**Effects**: Sets Local Sidereal Time

#### SY - Synchronize Coordinates
**Request**: `:SYsDD*MM:SS.HH:MM:SS#`
**Response**: `1` (success) or `0` (failure)
**Parameters**: Full RA and Dec coordinates
**Effects**: Alternative sync format with both coordinates in one command

### 12. Rate Control Commands

#### Rs - Set Slew Rate (by letter)
**Request**: `:Rss#`
**Parameters**: `s` = 'S' (slew/fastest), 'M' (find/medium-fast), 'C' (centering/medium), 'G' (guiding/slowest)
**Response**: None
**Notes**: Alternative to RC/RG/RM/RS commands

## Command Execution Contracts

### Synchronous Commands (Blocking)
These commands execute and return immediately:
- All Get commands (GR, GD, Gt, Gg, etc.)
- All Set commands (Sr, Sd, St, Sg, etc.)
- Halt commands (Q, Qe, Qw, Qn, Qs)
- Rate commands (RC, RG, RM, RS)

**Contract**: Response within 100ms

### Asynchronous Commands (Non-Blocking)
These commands initiate operations that continue in background:
- Slew commands (MS, Me, Mw, Mn, Ms)
- Tracking commands (T+, T-)

**Contract**: 
- Initial response within 100ms (acknowledge start)
- Use `:D#` to poll for completion
- Status available via Get commands during operation

### State Validation

**Invalid State Transitions**:
```
MS while already slewing → OK (continues current slew)
Sr without Sd (or vice versa) → Set accepted, but MS will fail
MS without Sr+Sd → Reject (no target)
T+ while slewing → OK (tracking starts after slew complete)
```

**Valid Command Sequences**:
```
# Basic Go-To
:Sr12:34:56#  → 1
:Sd+45*12:34# → 1
:MS#          → 0
:D#           → "" (polling)
:D#           → "#" (complete)

# Sync
:Sr12:34:56#  → 1
:Sd+45*12:34# → 1
:CM#          → "coordinates matched#"

# Manual Slew
:RC#          → (no response)
:Mn#          → (no response - starts slewing north)
:Qn#          → (no response - stops)

# Tracking
:T+#          → (no response - starts tracking)
:GR#          → "12:34:57#" (position updates)
:T-#          → (no response - stops tracking)
```

## Error Handling

**Protocol Errors** (logged, not returned to client):
- Missing `:` start marker
- Missing `#` terminator
- Invalid command format
- Buffer overflow (command > 256 bytes)

**Semantic Errors** (indicated by response):
- Invalid coordinates: `0` response from Set commands
- No target: `1` + `"Object Below Horizon#"` from MS
- Invalid state: Varies by command

**All errors logged** via Zephyr logging:
```cpp
LOG_ERR("LX200: Invalid RA format: '%s'", input);
LOG_WRN("LX200: MS command with no target set");
LOG_ERR("LX200: Command buffer overflow: %zu bytes", len);
```

## Testing Contracts

### Unit Test Requirements
Each command class must have tests for:
1. Valid input parsing
2. Invalid input rejection  
3. Boundary conditions
4. State validation
5. Response formatting

### Integration Test Requirements
Full command sequences must work:
1. Goto sequence (Sr, Sd, MS, D)
2. Sync sequence (Sr, Sd, CM)
3. Tracking (T+, GR polling, T-)
4. Manual slew (RC, Mn, Qn)
5. Emergency stop (Q during any operation)

### Compatibility Test Requirements
Verify against real clients:
1. Stellarium connection and goto
2. ASCOM driver compatibility
3. INDI client compatibility
4. SkySafari operation

## Next Steps

This contract document defines the complete LX200 protocol interface. Implementation will follow TDD:
1. Write tests for each command (one test file per command family)
2. Implement command classes to pass tests
3. Integration tests for command sequences
4. Compatibility tests with real astronomy software

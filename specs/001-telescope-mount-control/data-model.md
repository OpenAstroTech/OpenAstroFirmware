# Data Model: Telescope Mount Control System

**Feature**: Telescope Mount Control System  
**Phase**: 1 - Design & Contracts  
**Date**: 2025-10-12

## Overview

This document defines the core data structures and their relationships for the telescope mount control system. All structures use C++20 with strong typing, immutability where appropriate, and clear ownership semantics.

## Core Entities

### 1. Celestial Coordinates

**Purpose**: Represent positions in the night sky using Right Ascension and Declination

```cpp
// Strong types for safety
struct RightAscension {
    int hours;        // 0-23
    int minutes;      // 0-59
    double seconds;   // 0.0-59.999...
    
    // Validation
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return hours >= 0 && hours < 24 &&
               minutes >= 0 && minutes < 60 &&
               seconds >= 0.0 && seconds < 60.0;
    }
    
    // Conversions
    [[nodiscard]] constexpr double toRadians() const noexcept;
    [[nodiscard]] constexpr double toHours() const noexcept;
    [[nodiscard]] std::string toString() const;  // "HH:MM:SS" or "HH:MM:SS.S"
    
    // Parsing
    [[nodiscard]] static std::optional<RightAscension> fromString(std::string_view s);
};

struct Declination {
    bool positive;         // true = north, false = south
    int degrees;           // 0-90
    int arcminutes;        // 0-59
    double arcseconds;     // 0.0-59.999...
    
    // Validation
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return degrees >= 0 && degrees <= 90 &&
               arcminutes >= 0 && arcminutes < 60 &&
               arcseconds >= 0.0 && arcseconds < 60.0;
    }
    
    // Conversions
    [[nodiscard]] constexpr double toRadians() const noexcept;
    [[nodiscard]] constexpr double toDegrees() const noexcept;
    [[nodiscard]] std::string toString() const;  // "sDD*MM:SS" or "sDD*MM'SS"
    
    // Parsing
    [[nodiscard]] static std::optional<Declination> fromString(std::string_view s);
};

// Immutable celestial coordinate
class CelestialCoordinate {
    RightAscension ra_;
    Declination dec_;
    
public:
    constexpr CelestialCoordinate(RightAscension ra, Declination dec) noexcept
        : ra_(ra), dec_(dec) {}
    
    [[nodiscard]] constexpr RightAscension ra() const noexcept { return ra_; }
    [[nodiscard]] constexpr Declination dec() const noexcept { return dec_; }
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return ra_.isValid() && dec_.isValid();
    }
    
    // Equality
    [[nodiscard]] constexpr bool operator==(const CelestialCoordinate&) const noexcept = default;
};
```

**Validation Rules**:
- RA: 0h ≤ hours < 24h, 0m ≤ minutes < 60m, 0s ≤ seconds < 60s
- Dec: 0° ≤ degrees ≤ 90°, 0' ≤ arcminutes < 60', 0" ≤ arcseconds < 60"
- Dec sign: stored separately from magnitude (positive = north of celestial equator)

**State Transitions**: None - coordinates are immutable value types

### 2. Mount Position

**Purpose**: Current physical position of the mount axes

```cpp
// Motor position in steps (raw hardware units)
struct MotorPosition {
    int64_t ra_steps;      // Right Ascension axis position
    int64_t dec_steps;     // Declination axis position
    
    constexpr MotorPosition() noexcept : ra_steps(0), dec_steps(0) {}
    constexpr MotorPosition(int64_t ra, int64_t dec) noexcept 
        : ra_steps(ra), dec_steps(dec) {}
};

// Mount position with both motor and celestial coordinates
class MountPosition {
    MotorPosition motor_pos_;
    CelestialCoordinate celestial_pos_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    MountPosition(MotorPosition motor, CelestialCoordinate celestial)
        : motor_pos_(motor), 
          celestial_pos_(celestial),
          timestamp_(std::chrono::system_clock::now()) {}
    
    [[nodiscard]] const MotorPosition& motorPosition() const noexcept { 
        return motor_pos_; 
    }
    [[nodiscard]] const CelestialCoordinate& celestialPosition() const noexcept { 
        return celestial_pos_; 
    }
    [[nodiscard]] auto timestamp() const noexcept { return timestamp_; }
};
```

**Relationships**:
- Motor position (steps) ↔ Celestial coordinate via calibration model
- Timestamp tracks when position was recorded

### 3. Tracking State

**Purpose**: Manage the mount's operational mode and tracking behavior

```cpp
enum class TrackingMode {
    IDLE,           // No tracking, motors disabled
    SLEWING,        // Moving to target position
    TRACKING,       // Following sidereal motion
    GUIDING,        // Fine corrections during tracking
    STOPPED         // Emergency stop, motors locked
};

class TrackingState {
    TrackingMode mode_;
    CelestialCoordinate target_;
    double tracking_rate_arcsec_per_sec_;  // Usually 15.041067 (sidereal)
    std::optional<CelestialCoordinate> guide_correction_;
    
public:
    TrackingState() noexcept 
        : mode_(TrackingMode::IDLE),
          target_(RightAscension{0,0,0.0}, Declination{true,0,0,0.0}),
          tracking_rate_arcsec_per_sec_(15.041067) {}
    
    [[nodiscard]] TrackingMode mode() const noexcept { return mode_; }
    [[nodiscard]] const CelestialCoordinate& target() const noexcept { return target_; }
    [[nodiscard]] double trackingRate() const noexcept { return tracking_rate_arcsec_per_sec_; }
    
    // State transitions
    void startSlewing(CelestialCoordinate target);
    void startTracking();
    void applyGuideCorrection(CelestialCoordinate correction);
    void stop();
    void emergencyStop();
};
```

**State Transitions**:
```
IDLE → SLEWING (gotoCoordinate)
SLEWING → TRACKING (slew complete)
TRACKING → GUIDING (guide correction applied)
GUIDING → TRACKING (guide correction complete)
TRACKING → STOPPED (stop command)
SLEWING → STOPPED (stop command)
ANY → STOPPED (emergency stop)
STOPPED → IDLE (reset)
```

**Validation Rules**:
- Cannot slew while already slewing
- Cannot start tracking without valid target
- Emergency stop has highest priority (preempts all other operations)

### 4. Calibration Data

**Purpose**: Store alignment information mapping motor positions to celestial coordinates

```cpp
struct AlignmentStar {
    CelestialCoordinate celestial;     // Known star position
    MotorPosition motor_observed;      // Where mount was pointing
    std::string name;                  // Star name for logging
};

class CalibrationModel {
    std::vector<AlignmentStar> alignment_stars_;  // 2-3 reference stars
    Eigen::Matrix3d rotation_matrix_;             // Coordinate transformation
    Location observer_location_;                  // Lat/long for calculations
    std::chrono::system_clock::time_point time_;  // When calibration performed
    bool is_valid_;
    
public:
    CalibrationModel() : is_valid_(false) {}
    
    // Add alignment star
    void addAlignmentStar(AlignmentStar star);
    
    // Compute transformation (requires 2-3 stars)
    bool computeAlignment();
    
    // Conversions using calibration
    [[nodiscard]] std::optional<CelestialCoordinate> 
        motorToCelestial(MotorPosition motor) const;
    
    [[nodiscard]] std::optional<MotorPosition> 
        celestialToMotor(CelestialCoordinate celestial) const;
    
    [[nodiscard]] bool isValid() const noexcept { return is_valid_; }
    [[nodiscard]] size_t alignmentStarCount() const noexcept { 
        return alignment_stars_.size(); 
    }
    
    // Persistence
    bool saveToSettings();
    bool loadFromSettings();
};
```

**Validation Rules**:
- Minimum 2 alignment stars required (3 preferred for better accuracy)
- Stars must be at least 30° apart in the sky
- Calibration expires after 24 hours (Earth's rotation changes relationship)
- Must recalibrate if observer location changes

### 5. Mount Configuration

**Purpose**: Hardware-specific parameters loaded from device tree

```cpp
struct MountConfig {
    // Motor specifications
    uint32_t ra_steps_per_revolution;      // Total steps for 360° rotation
    uint32_t dec_steps_per_revolution;
    uint32_t ra_max_speed_steps_per_sec;   // Maximum safe speed
    uint32_t dec_max_speed_steps_per_sec;
    uint32_t acceleration_steps_per_sec2;  // Acceleration limit
    
    // Mechanical limits
    int64_t ra_min_position_steps;         // Software limits
    int64_t ra_max_position_steps;
    int64_t dec_min_position_steps;
    int64_t dec_max_position_steps;
    
    // Tracking parameters
    double sidereal_rate_arcsec_per_sec;   // 15.041067 for Earth
    double lunar_rate_arcsec_per_sec;      // 14.515 for Moon tracking
    double solar_rate_arcsec_per_sec;      // 15.0 for Sun tracking
    
    // Calibration
    bool backlash_compensation_enabled;
    uint32_t ra_backlash_steps;            // Steps to compensate
    uint32_t dec_backlash_steps;
    
    // Load from device tree at startup
    static MountConfig loadFromDeviceTree();
    
    // Validation
    [[nodiscard]] bool isValid() const noexcept;
};
```

**Source**: Device tree properties (read-only after boot)

**Validation Rules**:
- Steps per revolution must be > 0
- Max speeds must be achievable by hardware
- Software limits must be within mechanical range
- Backlash must be < 1% of steps per revolution

### 6. Observer Location

**Purpose**: Geographic location for celestial calculations

```cpp
struct Location {
    double latitude_degrees;   // -90 to +90, north positive
    double longitude_degrees;  // -180 to +180, east positive
    double elevation_meters;   // Above sea level (optional, default 0)
    std::string timezone;      // IANA timezone name
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return latitude_degrees >= -90.0 && latitude_degrees <= 90.0 &&
               longitude_degrees >= -180.0 && longitude_degrees <= 180.0 &&
               elevation_meters >= -500.0 && elevation_meters <= 9000.0;
    }
    
    // Persistence
    bool saveToSettings();
    static std::optional<Location> loadFromSettings();
};
```

**Validation Rules**:
- Latitude: -90° (south pole) to +90° (north pole)
- Longitude: -180° to +180° (meridian convention)
- Elevation: -500m (Death Valley) to 9000m (highest observatories)

### 7. LX200 Command/Response

**Purpose**: Encapsulate LX200 protocol messages

```cpp
// Command base class (polymorphic)
class LX200Command {
public:
    virtual ~LX200Command() = default;
    
    // Execute command on mount controller
    virtual LX200Response execute(MountController& mount) = 0;
    
    // Command properties
    [[nodiscard]] virtual bool requiresParameter() const noexcept = 0;
    [[nodiscard]] virtual std::string_view commandName() const noexcept = 0;
};

// Response types
enum class LX200ResponseType {
    ACKNOWLEDGE,     // "1" - command accepted
    REJECT,          // "0" - command rejected
    STRING,          // Text response (coordinates, status, etc.)
    NONE             // No response expected
};

class LX200Response {
    LX200ResponseType type_;
    std::string data_;
    
public:
    explicit LX200Response(LX200ResponseType type, std::string data = "")
        : type_(type), data_(std::move(data)) {}
    
    [[nodiscard]] LX200ResponseType type() const noexcept { return type_; }
    [[nodiscard]] const std::string& data() const noexcept { return data_; }
    [[nodiscard]] std::string_view view() const noexcept { return data_; }
    
    // Formatted output for UART
    [[nodiscard]] std::string format() const;
    
    // Factory methods
    static LX200Response acknowledge() { return LX200Response(LX200ResponseType::ACKNOWLEDGE, "1"); }
    static LX200Response reject() { return LX200Response(LX200ResponseType::REJECT, "0"); }
    static LX200Response string(std::string s) { return LX200Response(LX200ResponseType::STRING, std::move(s)); }
    static LX200Response none() { return LX200Response(LX200ResponseType::NONE); }
};
```

**Command Hierarchy**:
```
LX200Command (abstract base)
├── SyncCommand (CM - Synchronize)
├── GetRACommand (GR - Get Right Ascension)
├── GetDecCommand (GD - Get Declination)
├── SetRACommand (Sr - Set target RA)
├── SetDecCommand (Sd - Set target Dec)
├── SlewCommand (MS - Slew to target)
├── HaltCommand (Q - Quit motion)
├── TrackingOnCommand (T - Enable tracking)
├── TrackingOffCommand (T - Disable tracking)
└── ... (50+ command types)
```

**Ownership**: Commands created by parser, executed immediately, then destroyed

## Data Relationships

```
Observer Location
       │
       ├─── Used by ───> Calibration Model
       │                        │
       │                        │ transforms
       │                        ↓
       │                  Mount Position ←─── read/write ───┐
       │                        │                            │
       └─── Used by ───> Tracking State ───> controls ──> Mount Controller
                                │                            │
                          LX200 Command ────> executes ─────┘
                                │
                          LX200 Response ←── produces ───────┘
```

## Persistence Strategy

**Persistent Data** (survives power cycle):
- Calibration model → Zephyr settings subsystem
- Observer location → Zephyr settings subsystem
- Last known position → Zephyr settings subsystem (for parking)

**Volatile Data** (reset on power cycle):
- Current tracking state (always starts in IDLE)
- Motor positions (re-homed on startup)
- LX200 command queue

**Configuration Data** (read-only, from device tree):
- Mount hardware configuration
- Motor specifications
- Pin assignments

## Memory Layout

**Static Allocation** (compile-time known):
```cpp
// Singleton mount controller (static lifetime)
MountController g_mount_controller;

// Configuration (loaded at startup, never changes)
constexpr size_t CONFIG_SIZE = sizeof(MountConfig);  // ~200 bytes

// State (single instance)
constexpr size_t STATE_SIZE = sizeof(TrackingState);  // ~100 bytes
```

**Stack Allocation** (function scope):
```cpp
// Coordinates created/destroyed in functions
void handleCommand() {
    CelestialCoordinate coord = parseCoordinate(input);  // Stack allocated
    auto response = executeCommand(coord);  // Stack allocated
}  // Automatic cleanup
```

**Dynamic Allocation** (minimal, non-critical paths only):
```cpp
// Command objects (short-lived, created per command)
std::unique_ptr<LX200Command> cmd = parser.parse(input);  // Heap allocated
auto response = cmd->execute(mount);  // Executed immediately
// cmd destroyed when unique_ptr goes out of scope
```

**Total Memory Budget**:
- Mount state: ~4KB
- Calibration data: ~2KB
- LX200 buffers: ~2KB
- Stack per thread: ~4KB × 3 threads = 12KB
- **Total**: ~20KB (well within 128KB constraint)

## Thread Safety

**Shared Mutable State**:
- Mount position: Protected by Zephyr mutex
- Tracking state: Protected by Zephyr mutex
- Calibration model: Read-only after computation (no mutex needed)

**Immutable Data** (thread-safe by design):
- CelestialCoordinate
- MountConfig
- Observer Location (after initial setup)

**Thread-Local Data**:
- LX200 command parsing buffer (per command thread)
- Motor control calculations (per tracking thread)

## Validation Summary

All data structures include validation:
- **Compile-time**: `constexpr` functions validate at compile time where possible
- **Runtime**: `isValid()` methods check invariants
- **Parse-time**: `fromString()` functions return `std::optional<T>` (nullopt on error)
- **Execution-time**: Commands return error codes if preconditions not met

Next: Define LX200 protocol contracts in `contracts/` directory

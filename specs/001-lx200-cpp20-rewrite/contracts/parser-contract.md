# Contract: Parser Interface

**Feature**: 001-lx200-cpp20-rewrite  
**Contract Type**: Unit Interface Contract  
**Purpose**: Define the expected behavior of the LX200Parser class

---

## Interface Definition

```cpp
class LX200Parser {
public:
    // Constructor
    LX200Parser();
    
    // State management
    void reset();
    ParseResult feed_character(char c);
    std::optional<Command> get_command();
    
    // Precision mode
    PrecisionMode get_precision() const;
    void set_precision(PrecisionMode mode);
};
```

---

## Contract Tests

### TC-001: Parser Initialization

**Given**: A new LX200Parser is created

**Then**:
- Buffer is empty (length = 0)
- Precision mode is High
- Command is not complete
- get_command() returns std::nullopt

**Test Code**:
```cpp
TEST(ParserContract, Initialization) {
    LX200Parser parser;
    
    ASSERT_EQ(parser.get_precision(), PrecisionMode::High);
    ASSERT_FALSE(parser.get_command().has_value());
}
```

---

### TC-002: Parser Reset

**Given**: Parser has partial command in buffer

**When**: reset() is called

**Then**:
- Buffer is cleared
- Command complete flag is false
- Precision mode is unchanged
- Next get_command() returns std::nullopt

**Test Code**:
```cpp
TEST(ParserContract, Reset) {
    LX200Parser parser;
    
    parser.feed_character(':');
    parser.feed_character('G');
    parser.reset();
    
    ASSERT_FALSE(parser.get_command().has_value());
}
```

---

### TC-003: Feed Valid Command Characters

**Given**: Parser is in initial state

**When**: Characters `:`, `G`, `R`, `#` are fed sequentially

**Then**:
- First 3 characters return ParseResult::Incomplete
- Final `#` returns ParseResult::Success
- get_command() returns Command with name="GR", no parameters

**Test Code**:
```cpp
TEST(ParserContract, FeedValidCommand) {
    LX200Parser parser;
    
    ASSERT_EQ(parser.feed_character(':'), ParseResult::Incomplete);
    ASSERT_EQ(parser.feed_character('G'), ParseResult::Incomplete);
    ASSERT_EQ(parser.feed_character('R'), ParseResult::Incomplete);
    ASSERT_EQ(parser.feed_character('#'), ParseResult::Success);
    
    auto cmd = parser.get_command();
    ASSERT_TRUE(cmd.has_value());
    ASSERT_EQ(cmd->name, "GR");
    ASSERT_TRUE(cmd->parameters.empty());
}
```

---

### TC-004: Missing Prefix

**Given**: Parser is in initial state

**When**: First character is not ':'

**Then**:
- feed_character() returns ParseResult::InvalidPrefix
- Buffer is reset
- Subsequent correct command can be parsed

**Test Code**:
```cpp
TEST(ParserContract, MissingPrefix) {
    LX200Parser parser;
    
    ASSERT_EQ(parser.feed_character('G'), ParseResult::InvalidPrefix);
    
    // Should recover
    ASSERT_EQ(parser.feed_character(':'), ParseResult::Incomplete);
}
```

---

### TC-005: Buffer Overflow

**Given**: Parser has received partial command

**When**: More than MAX_COMMAND_LENGTH characters are fed

**Then**:
- feed_character() returns ParseResult::BufferOverflow
- Buffer is reset for recovery
- Parser can accept new command

**Test Code**:
```cpp
TEST(ParserContract, BufferOverflow) {
    LX200Parser parser;
    
    parser.feed_character(':');
    ParseResult result = ParseResult::Incomplete;
    
    // Feed MAX_COMMAND_LENGTH characters
    for (size_t i = 0; i < MAX_COMMAND_LENGTH; ++i) {
        result = parser.feed_character('A');
    }
    
    ASSERT_EQ(result, ParseResult::BufferOverflow);
}
```

---

### TC-006: Command with Parameters

**Given**: Parser is in initial state

**When**: Command `:Sr12:34:56#` is fed

**Then**:
- ParseResult::Success returned on `#`
- Command name = "Sr"
- Command parameters = "12:34:56"
- Command family = CommandFamily::SetInfo

**Test Code**:
```cpp
TEST(ParserContract, CommandWithParameters) {
    LX200Parser parser;
    std::string_view cmd = ":Sr12:34:56#";
    
    ParseResult result;
    for (char c : cmd.substr(0, cmd.length()-1)) {
        result = parser.feed_character(c);
    }
    result = parser.feed_character('#');
    
    ASSERT_EQ(result, ParseResult::Success);
    
    auto command = parser.get_command();
    ASSERT_TRUE(command.has_value());
    ASSERT_EQ(command->name, "Sr");
    ASSERT_EQ(command->parameters, "12:34:56");
}
```

---

### TC-007: Precision Mode Toggle

**Given**: Parser is in High precision mode

**When**: set_precision(PrecisionMode::Low) is called

**Then**:
- get_precision() returns PrecisionMode::Low
- Mode persists across commands

**Test Code**:
```cpp
TEST(ParserContract, PrecisionModeToggle) {
    LX200Parser parser;
    
    ASSERT_EQ(parser.get_precision(), PrecisionMode::High);
    
    parser.set_precision(PrecisionMode::Low);
    
    ASSERT_EQ(parser.get_precision(), PrecisionMode::Low);
}
```

---

### TC-008: Multiple Sequential Commands

**Given**: Parser is in initial state

**When**: Two commands are fed: `:GR#:Gd#`

**Then**:
- First command (GR) is parsed successfully
- get_command() returns GR command
- Second command (Gd) is parsed successfully
- get_command() returns Gd command

**Test Code**:
```cpp
TEST(ParserContract, MultipleCommands) {
    LX200Parser parser;
    
    // Feed first command
    for (char c : std::string_view(":GR#")) {
        parser.feed_character(c);
    }
    
    auto cmd1 = parser.get_command();
    ASSERT_TRUE(cmd1.has_value());
    ASSERT_EQ(cmd1->name, "GR");
    
    // Feed second command
    for (char c : std::string_view(":Gd#")) {
        parser.feed_character(c);
    }
    
    auto cmd2 = parser.get_command();
    ASSERT_TRUE(cmd2.has_value());
    ASSERT_EQ(cmd2->name, "Gd");
}
```

---

### TC-009: Empty Command

**Given**: Parser is in initial state

**When**: Only `:#` is fed

**Then**:
- ParseResult::InvalidCommand is returned
- get_command() returns std::nullopt

**Test Code**:
```cpp
TEST(ParserContract, EmptyCommand) {
    LX200Parser parser;
    
    parser.feed_character(':');
    auto result = parser.feed_character('#');
    
    ASSERT_EQ(result, ParseResult::InvalidCommand);
    ASSERT_FALSE(parser.get_command().has_value());
}
```

---

### TC-010: Partial Command Buffering

**Given**: Parser is in initial state

**When**: Partial command `:GR` is fed (no terminator)

**Then**:
- All characters return ParseResult::Incomplete
- get_command() returns std::nullopt
- When `#` is later fed, command completes

**Test Code**:
```cpp
TEST(ParserContract, PartialCommandBuffering) {
    LX200Parser parser;
    
    parser.feed_character(':');
    parser.feed_character('G');
    parser.feed_character('R');
    
    ASSERT_FALSE(parser.get_command().has_value());
    
    parser.feed_character('#');
    
    auto cmd = parser.get_command();
    ASSERT_TRUE(cmd.has_value());
    ASSERT_EQ(cmd->name, "GR");
}
```

---

## Contract Invariants

1. **Buffer Safety**: Parser MUST never write beyond buffer bounds
2. **State Consistency**: After any operation, parser remains in valid state
3. **Recovery**: After error, reset() MUST restore parser to working state
4. **Determinism**: Same input sequence MUST produce same results
5. **No Memory Leaks**: Parser MUST not allocate dynamic memory

---

## Performance Contracts

- **Feed Character**: <1μs per character (no allocation, simple state update)
- **Get Command**: <1μs (returns reference, no copying)
- **Reset**: <10μs (memset buffer)

---

## Error Handling Contracts

- **Invalid Input**: MUST NOT crash or corrupt state
- **Buffer Overflow**: MUST return error and reset, allowing recovery
- **Multiple Errors**: MUST handle repeated invalid input without degradation

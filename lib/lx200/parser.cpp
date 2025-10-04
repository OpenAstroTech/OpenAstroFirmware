/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lx200/lx200.hpp>
#include <cstring>

namespace lx200 {

/* ========================================================================
 * ParserState Implementation
 * ======================================================================== */

ParserState::ParserState() noexcept
    : buffer_{}
    , buffer_length_{0}
    , command_complete_{false}
    , precision_{PrecisionMode::High}
{
}

void ParserState::reset() noexcept
{
    buffer_length_ = 0;
    command_complete_ = false;
    // Note: precision_ is NOT reset - it persists across commands
}

ParseResult ParserState::feed_character(char c) noexcept
{
    // Check for buffer overflow
    if (buffer_length_ >= MAX_COMMAND_LENGTH) {
        return ParseResult::ErrorBufferFull;
    }
    
    // First character must be ':'
    if (buffer_length_ == 0) {
        if (c != ':') {
            return ParseResult::ErrorInvalidFormat;
        }
        buffer_[buffer_length_++] = c;
        return ParseResult::Incomplete;
    }
    
    // Check for command terminator
    if (c == '#') {
        // Empty command (just ":#") is invalid
        if (buffer_length_ == 1) {
            return ParseResult::ErrorInvalidFormat;
        }
        
        buffer_[buffer_length_++] = c;
        command_complete_ = true;
        return ParseResult::Success;
    }
    
    // Accumulate character
    buffer_[buffer_length_++] = c;
    return ParseResult::Incomplete;
}

bool ParserState::is_command_ready() const noexcept
{
    return command_complete_;
}

std::optional<Command> ParserState::get_command() noexcept
{
    if (!command_complete_) {
        return std::nullopt;
    }
    
    // Extract command string (skip ':' prefix and '#' terminator)
    std::string_view full_command(buffer_.data() + 1, buffer_length_ - 2);
    
    // Split into name and parameters
    std::string_view name, params;
    parse_command_parts(name, params);
    
    // Identify command family based on full command name
    CommandFamily family = CommandFamily::Unknown;
    
    // Special cases for date/time commands (semantic grouping)
    if (name == "GC" || name == "GL" || name == "Ga" || name == "Gc" ||
        name == "SC" || name == "SL" || name == "SG" || name == "SH") {
        family = CommandFamily::DateTime;
    }
    // Special case: gT (update time from GPS) is GetInfo, not GPS
    else if (name == "gT") {
        family = CommandFamily::GetInfo;
    }
    // Default: use first character mapping
    else {
        family = identify_family(name.empty() ? '\0' : name[0]);
    }
    
    // Create command
    Command cmd{
        .family = family,
        .name = name,
        .parameters = params
    };
    
    // Reset for next command (but preserve precision)
    // NOTE: The Command's string_views reference buffer_ which will be
    // reused immediately. Caller MUST consume the command before calling
    // feed_character() again. See Command struct documentation.
    reset();
    
    return cmd;
}

CommandFamily ParserState::identify_family(char first_char) const noexcept
{
    // Direct character-to-family mapping
    switch (first_char) {
        case 'A': return CommandFamily::Alignment;
        case 'B': return CommandFamily::Backup;
        case 'C': return CommandFamily::DateTime;
        case 'D': return CommandFamily::Distance;
        case 'F': return CommandFamily::Focus;
        case 'G': return CommandFamily::GetInfo;
        case 'g': return CommandFamily::GPS;
        case 'h': return CommandFamily::Home;
        case 'H': return CommandFamily::Home;  // Hour angle also maps to Home
        case 'I': return CommandFamily::Initialize;
        case 'L': return CommandFamily::Library;
        case 'M': return CommandFamily::Movement;
        case 'P': return CommandFamily::Precision;
        case 'Q': return CommandFamily::Quit;
        case 'R': return CommandFamily::Rate;
        case 'S': return CommandFamily::SetInfo;
        case 'T': return CommandFamily::Tracking;
        case 'U': return CommandFamily::User;
        case 'X': return CommandFamily::Extended;
        default:  return CommandFamily::Unknown;
    }
}

void ParserState::parse_command_parts(std::string_view& name, std::string_view& params) const noexcept
{
    // Get full command (excluding ':' and '#')
    std::string_view full_command(buffer_.data() + 1, buffer_length_ - 2);
    
    if (full_command.empty()) {
        name = std::string_view{};
        params = std::string_view{};
        return;
    }
    
    // Lookup table: command families that take parameters after 2-char name
    // This replaces the if-else chain for better maintainability
    static constexpr char PARAMETER_FAMILIES[] = {
        'S',  // SetInfo commands (Sr, Sd, SC, SL, etc.)
        'R',  // Rate commands (R0-R9)
        'T',  // Tracking commands (T+, T-)
        'F',  // Focus commands (F+, F-)
        'B',  // Reticle commands (B+, B-)
        'g',  // GPS commands (some variants)
        'L',  // Library commands (some variants)
    };
    
    if (full_command.length() > 2) {
        char first = full_command[0];
        
        // Check if first character is in parameter families
        for (char family : PARAMETER_FAMILIES) {
            if (first == family) {
                // Standard pattern: 2-char command name + parameters
                name = full_command.substr(0, 2);
                params = full_command.substr(2);
                return;
            }
        }
    }
    
    // No parameters - entire string is command name
    name = full_command;
    params = std::string_view{};
}

} // namespace lx200

/*
 * Copyright (c) 2025, OpenAstroTech
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file lx200.cpp
 * @brief LX200 Protocol Implementation - Main Module
 * 
 * This file serves as the main entry point for the LX200 C++20
 * implementation. The actual implementation is split across:
 * - parser.cpp: ParserState class and command parsing
 * - coordinates.cpp: Coordinate parsing functions
 * 
 * This architecture enables:
 * - Parallel compilation
 * - Clear separation of concerns
 * - Easier testing and maintenance
 */

#include <lx200/lx200.hpp>

namespace lx200 {

// Implementation note:
// All functions are implemented in parser.cpp and coordinates.cpp
// This file exists to provide a central compilation unit if needed
// and to ensure the lx200 namespace is properly established.

} // namespace lx200

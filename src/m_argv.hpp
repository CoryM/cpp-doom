//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  Nil.
//


#ifndef __M_ARGV__
#define __M_ARGV__

#include "doomtype.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

//
// MISC
//
void M_SetArgument(int newArgC, char **newArgV);

// Returns the position of the given parameter
// in the arg list (0 if not found).
auto M_CheckParm(const char *check) -> int;

// Same as M_CheckParm, but checks that num_args arguments are available
// following the specified argument.
auto M_CheckParmWithArgs(const char *check, int num_args) -> int;
auto M_GetArgument(int arg) -> std::string_view;
auto M_GetArgumentAsInt(int arg) -> int;
auto M_GetArgumentCount() -> int;

void M_FindResponseFile();
void M_AddLooseFiles();

// Parameter has been specified?

auto M_ParmExists(const char *check) -> bool;

// Get name of executable used to run this program:

auto M_GetExecutableName() -> const char *;

class c_Arguments {
private:
    std::vector<std::string> m_Args;

public:
    c_Arguments() = default;
    c_Arguments(int newArgC, char **newArgV);
    ~c_Arguments() = default;
    void importArguments(const std::span<char *> &arrayOfArgs);
    auto at(const size_t pos) -> std::string;
};

static c_Arguments myArgs;

#endif

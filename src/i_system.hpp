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
//	System specific interface stuff.
//


#ifndef __I_SYSTEM__
#define __I_SYSTEM__


#include "common.hpp"
#include "d_event.hpp"
#include "d_ticcmd.hpp"

#include <string>
#include <string_view>


typedef void (*atexit_func_t)();

// Called by DoomMain.
void I_Init();

// Called by startup code
// to get the amount of memory to malloc
// for the zone management.
auto I_ZoneBase(size_t *size) -> byte *;

auto I_ConsoleStdout() -> bool;


// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
auto I_BaseTiccmd() -> ticcmd_t *;


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
[[noreturn]] void I_Quit();

[[noreturn]] void S_Error(std::string_view error);

void I_Tactile(int on, int off, int total);

auto I_Realloc(void *ptr, size_t size) -> void *;

template <typename DataType>
auto i_realloc(void *ptr, size_t size)
{
    return static_cast<DataType>(I_Realloc(ptr, size));
}

auto I_GetMemoryValue(unsigned int offset, void *value, int size) -> bool;

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)

void I_AtExit(atexit_func_t func, bool run_if_error);

// Add all system-specific config file variable bindings.

void I_BindVariables();

// Print startup banner copyright message.

void I_PrintStartupBanner(const char *gamedescription);

// Print a centered text banner displaying the given string.

void I_PrintBanner(const char *text);

// Print a dividing line for startup banners.

void I_PrintDivider();

#endif // __I_SYSTEM_HPP__

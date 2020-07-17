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
//      System-specific timer interface
//


#ifndef __I_TIMER__
#define __I_TIMER__

#include <cstdint>

constexpr uint32_t TICRATE = 35;
constexpr uint32_t TICBase = 1000;

// Called by D_DoomLoop,
// returns current time in tics.
auto I_GetTime() -> uint32_t;

// returns current time in ms
auto I_GetTimeMS() -> uint32_t;

// Pause for a specified number of ms
void I_Sleep(uint32_t ms);

// Initialize timer
void I_InitTimer();

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

#endif

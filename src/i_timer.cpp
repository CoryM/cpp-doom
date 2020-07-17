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
//      Timer functions.
//

#include "doomtype.hpp"
#include "i_timer.hpp"

#include "SDL2/SDL.h"

//
// I_GetTime
// returns time in 1/35th second tics
//

static Uint32 basetime = 0;

auto I_GetTime() -> uint32_t
{
    auto ticks = static_cast<uint32_t>(SDL_GetTicks());

    if (basetime == 0)
    {
        basetime = ticks;
    }

    ticks -= basetime;

    return (ticks * TICRATE) / TICBase;
};

//
// Same as I_GetTime, but returns time in milliseconds
//

auto I_GetTimeMS() -> uint32_t
{
    auto ticks = static_cast<uint32_t>(SDL_GetTicks());

    if (basetime == 0)
    {
        basetime = ticks;
    }

    return ticks - basetime;
}

// Sleep for a specified number of ms

void I_Sleep(uint32_t ms)
{
    SDL_Delay(ms);
}

void I_WaitVBL(int count)
{
    constexpr uint32_t magicNumber = 70;
    I_Sleep((count * TICBase) / magicNumber);
}


void I_InitTimer()
{
    // initialize timer

#if SDL_VERSION_ATLEAST(2, 0, 5)
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
#endif
    SDL_Init(SDL_INIT_TIMER);
}

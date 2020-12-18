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
//  AutoMap module.
//

#ifndef __AMMAP_H__
#define __AMMAP_H__

#include "../m_cheat.hpp" // for cheatseq_t
struct event_t;

// Called by main loop.
auto AM_Responder(event_t *ev) -> bool;

// Called by main loop.
auto AM_Ticker() -> void;

// Called by main loop,
// called instead of view drawer if automap active.
auto AM_Drawer() -> void;

// Called to force the automap to quit
// if the level is completed while it is up.
auto AM_Stop() -> void;

namespace globals {
// Used by ST StatusBar stuff.
constexpr int AM_MSGHEADER  = 0x616D0000; // am??
constexpr int AM_MSGENTERED = 0x616D6500; // ame?
constexpr int AM_MSGEXITED  = 0x616D7800; // amx?

extern cheatseq_t cheat_amap;
}

#endif

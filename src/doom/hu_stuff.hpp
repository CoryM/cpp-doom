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
// DESCRIPTION:  Head up display
//

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "../i_timer.hpp" // for TICRATE
#include "../i_video.hpp" // for DELTAWIDTH

struct event_t;


//
// Globally visible constants.
//
constexpr int HU_FONTSTART = '!'; // the first font characters
constexpr int HU_FONTEND   = '_'; // the last font characters

// Calculate # of glyphs in font.
constexpr int HU_FONTSIZE = (HU_FONTEND - HU_FONTSTART + 1);

constexpr int HU_BROADCAST = 5;

#define HU_MSGX (0 - DELTAWIDTH)
constexpr int HU_MSGY      = 0;
constexpr int HU_MSGWIDTH  = 64; // in characters
constexpr int HU_MSGHEIGHT = 1;  // in lines

#define HU_MSGTIMEOUT (4 * TICRATE)

//
// HEADS UP TEXT
//

auto HU_Init() -> void;
auto HU_Start() -> void;

auto HU_Responder(event_t *ev) -> bool;

auto HU_Ticker() -> void;
auto HU_Drawer() -> void;
auto HU_dequeueChatChar() -> char;
auto HU_Erase() -> void;

extern char *chat_macros[10];

#endif

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
//    Nil.
//


#ifndef __M_BBOX__
#define __M_BBOX__

#include <climits>
#include <limits>

#include "m_fixed.hpp"

struct BoundingBox {
    fixed_t Top    = std::numeric_limits<fixed_t>::min();
    fixed_t Bottom = std::numeric_limits<fixed_t>::max();
    fixed_t Left   = std::numeric_limits<fixed_t>::max();
    fixed_t Right  = std::numeric_limits<fixed_t>::min();

    BoundingBox() = default;
    BoundingBox(fixed_t x, fixed_t y);
    ~BoundingBox() = default;

    auto ClearBox() -> void;
    auto AddToBox(fixed_t x, fixed_t y) -> void;
};


// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
}; // bbox coordinates

// Bounding box functions.
void M_ClearBox(fixed_t *box);

void M_AddToBox(fixed_t *box,
    fixed_t              x,
    fixed_t              y);


#endif

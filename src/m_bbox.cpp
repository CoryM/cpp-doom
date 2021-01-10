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
//	Main loop menu stuff.
//	Random number LUT.
//	Default Config File.
//	PCX Screenshots.
//


#include "m_bbox.hpp"

#include <algorithm>
#include <limits>

BoundingBox::BoundingBox(const fixed_t x, const fixed_t y)
    : Top(y)
    , Bottom(y)
    , Left(x)
    , Right(x)
{
}

auto BoundingBox::ClearBox() -> void
{
    Top    = std::numeric_limits<fixed_t>::min();
    Right  = std::numeric_limits<fixed_t>::min();
    Bottom = std::numeric_limits<fixed_t>::max();
    Left   = std::numeric_limits<fixed_t>::max();
}

auto BoundingBox::AddToBox(fixed_t x, fixed_t y) -> void
{
    Left   = std::min(Left, x);
    Right  = std::max(Right, x);
    Bottom = std::min(Bottom, y);
    Top    = std::max(Top, y);
}


void M_ClearBox(fixed_t *box)
{
    box[BOXTOP] = box[BOXRIGHT] = INT_MIN;
    box[BOXBOTTOM] = box[BOXLEFT] = INT_MAX;
}

void M_AddToBox(fixed_t *box,
    fixed_t              x,
    fixed_t              y)
{
    if (x < box[BOXLEFT])
        box[BOXLEFT] = x;
    else if (x > box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y < box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if (y > box[BOXTOP])
        box[BOXTOP] = y;
}

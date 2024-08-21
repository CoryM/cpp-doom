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
//      Refresh/rendering module, shared data struct definitions.
//


#ifndef V_PATCH_H
#define V_PATCH_H

// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
struct [[gnu::packed]] patch_t
    {
        short width; // bounding box size
        short height;
        short leftoffset;   // pixels to the left of origin
        short topoffset;    // pixels below the origin
        int   columnofs[8]; // only [width] used
        // the [0] is &columnofs[width]
    };

// posts are runs of non masked source pixels
struct [[gnu::packed]] post_t
    {
        uint8_t topdelta; // -1 is the last post in a column
        uint8_t length;   // length data bytes follows
    };

// column_t is a list of 0 or more post_t, (uint8_t)-1 terminated
typedef post_t column_t;

#endif

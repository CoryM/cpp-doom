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
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//


#ifndef __DOOMTYPE_HPP__
#define __DOOMTYPE_HPP__

#include "config.hpp"

// #define macros to provide functions missing in Windows.
// Outside Windows, we use strings.h for str[n]casecmp.

#include <cinttypes>
#include <climits>
#include <cstring>
#include <string_view>

//
// The packed attribute forces structures to be packed into the minimum
// space necessary.  If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size.  It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//

#define PACKEDATTR __attribute__((packed))

#define PRINTF_ATTR(fmt, first) __attribute__((format(printf, fmt, first)))
#define PRINTF_ARG_ATTR(x)      __attribute__((format_arg(x)))

#define PACKEDPREFIX

#define PACKED_STRUCT(...) PACKEDPREFIX struct __VA_ARGS__ PACKEDATTR

// C99 integer types; with gcc we just use this.  Other compilers
// should add conditional statements that define the C99 types.

// What is really wanted here is stdint.h; however, some old versions
// of Solaris don't have stdint.h and only have inttypes.h (the
// pre-standardisation version).  inttypes.h is also in the C99
// standard and defined to include stdint.h, so include this.


typedef uint8_t byte;
using pixel_t  = uint32_t;
using dpixel_t = int64_t;

#define DIR_SEPARATOR   '/'
#define DIR_SEPARATOR_S "/"
#define PATH_SEPARATOR  ':'

#ifndef arrlen
#define arrlen(array) (sizeof(array) / sizeof(*array))
#endif


#endif // __DOOMTYPE_HPP__

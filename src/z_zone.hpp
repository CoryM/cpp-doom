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
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//


#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <cstdio>

//
// ZONE MEMORY
// PU - purge tags.

enum class PU
{
    STATIC = 1, // static entire execution time
    SOUND,      // static while playing
    MUSIC,      // static while playing
    FREE,       // a free block
    LEVEL,      // static until level exited
    LEVSPEC,    // a special thinker in a level

    // Tags >= PU::PURGELEVEL are purgable whenever needed.

    PURGELEVEL,
    CACHE,

    // Total number of different tag types

    NUM_TAGS,
    ZERO         = 0,
    LASTPURGABLE = LEVSPEC
};


void Z_Init();
auto Z_Malloc(size_t size, PU tag, void *user) -> void *;
void Z_Free(void *ptr);
void Z_FreeTags(PU lowtag, PU hightag);
void Z_DumpHeap(PU lowtag, PU hightag);
void Z_FileDumpHeap(FILE *f);
void Z_CheckHeap();
void Z_ChangeTag2(void *ptr, PU tag, const char *file, int line);
void Z_ChangeUser(void *ptr, void **user);
auto Z_FreeMemory() -> int;
auto Z_ZoneSize() -> unsigned int;

template <typename DataType>
auto z_malloc(size_t size, PU tag, void *ptr)
{
    return static_cast<DataType>(Z_Malloc(size, tag, ptr));
}

//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#define Z_ChangeTag(p, t) \
    Z_ChangeTag2((p), (t), __FILE__, __LINE__)


#endif

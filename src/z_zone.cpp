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
//	Zone Memory Allocation. Neat.
//
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "z_zone.hpp"

#include "fmt/core.h"
#include <cstring>
#include <iostream>

//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cacheable block,
//  because it will get overwritten automatically if needed.
//

constexpr size_t  MEM_ALIGN = sizeof(void *);
constexpr int32_t ZONEID    = 0x1d4a11;

struct memblock_s {
    int                size; // including the header and possibly tiny fragments
    void **            user;
    PU                 tag; // PU::FREE if this is free
    int                id;  // should be ZONEID
    struct memblock_s *next;
    struct memblock_s *prev;
};


struct memzone_t {
    int         size;      // total bytes malloced, including header
    memblock_s  blocklist; // start / end cap for linked list
    memblock_s *rover;
};


static memzone_t *mainzone;
static bool       zero_on_free;
static bool       scan_on_free;


//
// Z_ClearZone
//
// Not used
// void Z_ClearZone(memzone_t *zone)
// {
//     memblock_s *block = (memblock_s *)((byte *)zone + sizeof(memzone_t));
//
//     // set the entire zone to one free block
//     zone->blocklist.next = block;
//     zone->blocklist.prev = block;
//
//     zone->blocklist.user = reinterpret_cast<void **>(zone);
//     zone->blocklist.tag  = PU::STATIC;
//     zone->rover          = block;
//
//     block->prev = block->next = &zone->blocklist;
//
//     // a free block.
//     block->tag = PU::FREE;
//
//     block->size = zone->size - sizeof(memzone_t);
// }


//
// Z_Init
//
void Z_Init()
{
    std::cout << "Entering Z_Init\n";
    size_t size = 0;

    mainzone       = reinterpret_cast<memzone_t *>(I_ZoneBase(&size));
    mainzone->size = size;

    // set the entire zone to one free block
    auto *block              = reinterpret_cast<memblock_s *>(reinterpret_cast<byte *>(mainzone) + sizeof(memzone_t));
    mainzone->blocklist.next = block;
    mainzone->blocklist.prev = block;

    mainzone->blocklist.user = reinterpret_cast<void **>(mainzone);
    mainzone->blocklist.tag  = PU::STATIC;
    mainzone->rover          = block;

    block->prev = block->next = &mainzone->blocklist;

    // free block
    block->tag = PU::FREE;

    block->size = mainzone->size - sizeof(memzone_t);

    // [Deliberately undocumented]
    // Zone memory debugging flag. If set, memory is zeroed after it is freed
    // to deliberately break any code that attempts to use it after free.
    //
    zero_on_free = M_ParmExists("-zonezero");

    // [Deliberately undocumented]
    // Zone memory debugging flag. If set, each time memory is freed, the zone
    // heap is scanned to look for remaining pointers to the freed block.
    //
    scan_on_free = M_ParmExists("-zonescan");
}

// Scan the zone heap for pointers within the specified range, and warn about
// any remaining pointers.
static void ScanForBlock(void *start, void *end)
{
    memblock_s *block = mainzone->blocklist.next;

    while (block->next != &mainzone->blocklist)
    {
        auto tag = block->tag;

        if (tag == PU::STATIC || tag == PU::LEVEL || tag == PU::LEVSPEC)
        {
            // Scan for pointers on the assumption that pointers are aligned
            // on word boundaries (word size depending on pointer size):
            auto **mem = reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + sizeof(memblock_s));
            int    len = (block->size - sizeof(memblock_s)) / sizeof(void *);

            for (int i = 0; i < len; ++i)
            {
                if (start <= mem[i] && mem[i] <= end)
                {
                    fprintf(stderr, "%p has dangling pointer into freed block %p (%p -> %p)\n",
                        mem, start, &mem[i], mem[i]);
                }
            }
        }

        block = block->next;
    }
}

//
// Z_Free
//
void Z_Free(void *ptr)
{
    auto *block = (memblock_s *)((byte *)ptr - sizeof(memblock_s));

    if (block->id != ZONEID)
    {
        S_Error("Z_Free: freed a pointer without ZONEID");
    }

    if (block->tag != PU::FREE && block->user != nullptr)
    {
        // clear the user's mark
        *block->user = nullptr;
    }

    // mark as free
    block->tag  = PU::FREE;
    block->user = nullptr;
    block->id   = 0;

    // If the -zonezero flag is provided, we zero out the block on free
    // to break code that depends on reading freed memory.
    if (zero_on_free)
    {
        memset(ptr, 0, block->size - sizeof(memblock_s));
    }
    if (scan_on_free)
    {
        ScanForBlock(ptr, reinterpret_cast<byte *>(ptr) + block->size - sizeof(memblock_s));
    }

    memblock_s *other = block->prev;

    if (other->tag == PU::FREE)
    {
        // merge with previous free block
        other->size += block->size;
        other->next       = block->next;
        other->next->prev = other;

        if (block == mainzone->rover)
        {
            mainzone->rover = other;
        }

        block = other;
    }

    other = block->next;
    if (other->tag == PU::FREE)
    {
        // merge the next free block onto the end
        block->size += other->size;
        block->next       = other->next;
        block->next->prev = block;

        if (other == mainzone->rover)
        {
            mainzone->rover = block;
        }
    }
}


//
// Z_Malloc
// You can pass a NULL user if the tag is < PU::PURGELEVEL.
//
constexpr int MINFRAGMENT = 64;


auto Z_Malloc(size_t size, PU tag, void *user) -> void *
{
    size = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);

    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += sizeof(memblock_s);

    // if there is a free block behind the rover,
    //  back up over them
    memblock_s *base = mainzone->rover;

    if (base->prev->tag == PU::FREE)
    {
        base = base->prev;
    }

    memblock_s *rover = base;
    memblock_s *start = base->prev;

    do
    {
        if (rover == start)
        {
            // scanned all the way around the list
            //          I_Error ("Z_Malloc: failed on allocation of %i bytes", size);

            // [crispy] allocate another zone twice as big
            Z_Init();

            base  = mainzone->rover;
            rover = base;
            start = base->prev;
        }

        if (rover->tag != PU::FREE)
        {
            if (rover->tag < PU::PURGELEVEL)
            {
                // hit a block that can't be purged,
                // so move base past it
                base = rover = rover->next;
            }
            else
            {
                // free the rover block (adding the size to base)

                // the rover can be the base block
                base = base->prev;
                Z_Free((byte *)rover + sizeof(memblock_s));
                base  = base->next;
                rover = base->next;
            }
        }
        else
        {
            rover = rover->next;
        }

    } while (base->tag != PU::FREE || base->size < size);


    // found a block big enough
    if (int extra = base->size - size; extra > MINFRAGMENT)
    {
        // there will be a free fragment after the allocated block
        auto *newblock = reinterpret_cast<memblock_s *>(reinterpret_cast<byte *>(base) + size);
        newblock->size = extra;

        newblock->tag        = PU::FREE;
        newblock->user       = nullptr;
        newblock->prev       = base;
        newblock->next       = base->next;
        newblock->next->prev = newblock;

        base->next = newblock;
        base->size = size;
    }

    if (user == nullptr && tag >= PU::PURGELEVEL)
    {
        S_Error("Z_Malloc: an owner is required for purgable blocks");
    }

    base->user = reinterpret_cast<void **>(user);
    base->tag  = tag;

    auto *result = reinterpret_cast<void *>(reinterpret_cast<byte *>(base) + sizeof(memblock_s));

    if (base->user != nullptr)
    {
        *base->user = result;
    }

    // next allocation will start looking here
    mainzone->rover = base->next;

    base->id = ZONEID;

    return result;
}


//
// Z_FreeTags
//
void Z_FreeTags(PU lowtag, PU hightag)
{
    memblock_s *next;
    for (auto *block = mainzone->blocklist.next;
         block != &mainzone->blocklist;
         block = next)
    {
        // get link before freeing
        next = block->next;

        // free block?
        if (block->tag == PU::FREE)
            continue;

        if (block->tag >= lowtag && block->tag <= hightag)
            Z_Free((byte *)block + sizeof(memblock_s));
    }
}


//
// Z_DumpHeap
// Note: TFileDumpHeap( stdout ) ?
//
void Z_DumpHeap(PU lowtag, PU hightag)
{
    memblock_s *block;

    printf("zone size: %i  location: %p\n",
        mainzone->size, mainzone);

    printf("tag range: %i to %i\n",
        lowtag, hightag);

    for (block = mainzone->blocklist.next;; block = block->next)
    {
        if (block->tag >= lowtag && block->tag <= hightag)
            printf("block:%p    size:%7i    user:%p    tag:%3i\n",
                block, block->size, block->user, block->tag);

        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte *)block + block->size != (byte *)block->next)
            printf("ERROR: block size does not touch the next block\n");

        if (block->next->prev != block)
            printf("ERROR: next block doesn't have proper back link\n");

        if (block->tag == PU::FREE && block->next->tag == PU::FREE)
            printf("ERROR: two consecutive free blocks\n");
    }
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap(FILE *f)
{
    memblock_s *block;

    fprintf(f, "zone size: %i  location: %p\n", mainzone->size, mainzone);

    for (block = mainzone->blocklist.next;; block = block->next)
    {
        fprintf(f, "block:%p    size:%7i    user:%p    tag:%3i\n",
            block, block->size, block->user, block->tag);

        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte *)block + block->size != (byte *)block->next)
            fprintf(f, "ERROR: block size does not touch the next block\n");

        if (block->next->prev != block)
            fprintf(f, "ERROR: next block doesn't have proper back link\n");

        if (block->tag == PU::FREE && block->next->tag == PU::FREE)
            fprintf(f, "ERROR: two consecutive free blocks\n");
    }
}


//
// Z_CheckHeap
//
void Z_CheckHeap(void)
{
    memblock_s *block;

    for (block = mainzone->blocklist.next;; block = block->next)
    {
        if (block->next == &mainzone->blocklist)
        {
            // all blocks have been hit
            break;
        }

        if ((byte *)block + block->size != (byte *)block->next)
            I_Error("Z_CheckHeap: block size does not touch the next block\n");

        if (block->next->prev != block)
            I_Error("Z_CheckHeap: next block doesn't have proper back link\n");

        if (block->tag == PU::FREE && block->next->tag == PU::FREE)
            I_Error("Z_CheckHeap: two consecutive free blocks\n");
    }
}


//
// Z_ChangeTag
//
void Z_ChangeTag2(void *ptr, PU tag, const char *file, int line)
{
    memblock_s *block;

    block = (memblock_s *)((byte *)ptr - sizeof(memblock_s));

    if (block->id != ZONEID)
        I_Error("%s:%i: Z_ChangeTag: block without a ZONEID!",
            file, line);

    if (tag >= PU::PURGELEVEL && block->user == NULL)
        I_Error("%s:%i: Z_ChangeTag: an owner is required "
                "for purgable blocks",
            file, line);

    block->tag = tag;
}

void Z_ChangeUser(void *ptr, void **user)
{
    memblock_s *block;

    block = (memblock_s *)((byte *)ptr - sizeof(memblock_s));

    if (block->id != ZONEID)
    {
        I_Error("Z_ChangeUser: Tried to change user for invalid block!");
    }

    block->user = user;
    *user       = ptr;
}


//
// Z_FreeMemory
//
int Z_FreeMemory(void)
{
    memblock_s *block;
    int         free;

    free = 0;

    for (block = mainzone->blocklist.next;
         block != &mainzone->blocklist;
         block = block->next)
    {
        if (block->tag == PU::FREE || block->tag >= PU::PURGELEVEL)
            free += block->size;
    }

    return free;
}

unsigned int Z_ZoneSize(void)
{
    return mainzone->size;
}

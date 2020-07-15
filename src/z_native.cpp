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
//	This is an implementation of the zone memory API which
//	uses native calls to malloc() and free().
//

#include "doomtype.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"

#include "fmt/core.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string_view>

#define ZONEID 0x1d4a11

struct memblock_s {
    int         id; // = ZONEID
    PU          tag;
    int         size;
    void **     user;
    memblock_s *prev;
    memblock_s *next;
};

// Linked list of allocated blocks for each tag type

static memblock_s *allocated_blocks[static_cast<size_t>(PU::NUM_TAGS)];

#ifdef TESTING

static int test_malloced = 0;

void *test_malloc(size_t size)
{
    int *result;

    if (test_malloced + size > 2 * 1024 * 1024)
    {
        return NULL;
    }

    test_malloced += size;

    result = malloc(size + sizeof(int));

    *result = size;

    return result + 1;
}

void test_free(void *data)
{
    int *i;

    i = ((int *)data) - 1;

    test_malloced -= *i;

    free(i);
}

#define malloc test_malloc
#define free   test_free

#endif /* #ifdef TESTING */


// Add a block into the linked list for its type.

static void Z_InsertBlock(memblock_s *block)
{
    block->prev                                    = nullptr;
    block->next                                    = allocated_blocks[static_cast<int>(block->tag)];
    allocated_blocks[static_cast<int>(block->tag)] = block;

    if (block->next != nullptr)
    {
        block->next->prev = block;
    }
}

// Remove a block from its linked list.

static void Z_RemoveBlock(memblock_s *block)
{
    // Unlink from list

    if (block->prev == NULL)
    {
        // Start of list

        allocated_blocks[static_cast<int>(block->tag)] = block->next;
    }
    else
    {
        block->prev->next = block->next;
    }

    if (block->next != NULL)
    {
        block->next->prev = block->prev;
    }
}

//
// Z_Init
//
void Z_Init(void)
{
    memset(allocated_blocks, 0, sizeof(allocated_blocks));
    printf("zone memory: Using native C allocator.\n");
}


//
// Z_Free
//
void Z_Free(void *ptr)
{
    auto *block = reinterpret_cast<memblock_s *>(reinterpret_cast<byte *>(ptr) - sizeof(memblock_s));

    if (block->id != ZONEID)
    {
        S_Error("Z_Free: freed a pointer without ZONEID");
    }

    if (block->tag != PU::FREE && block->user != nullptr)
    {
        // clear the user's mark

        *block->user = nullptr;
    }

    Z_RemoveBlock(block);

    // Free back to system

    free(block);
}

// Empty data from the cache list to allocate enough data of the size
// required.
//
// Returns true if any blocks were freed.

static auto ClearCache(int size) -> bool
{
    auto *block = allocated_blocks[static_cast<int>(PU::CACHE)];

    if (block == nullptr)
    {
        // Cache is already empty.

        return false;
    }

    // Search to the end of the PU::CACHE list.  The blocks at the end
    // of the list are the ones that have been free for longer and
    // are more likely to be unneeded now.

    while (block->next != nullptr)
    {
        block = block->next;
    }

    //printf("out of memory; cleaning out the cache: %i\n", test_malloced);

    // Search backwards through the list freeing blocks until we have
    // freed the amount of memory required.

    auto remaining = size;

    while (remaining > 0)
    {
        if (block == nullptr)
        {
            // No blocks left to free; we've done our best.

            break;
        }

        auto *next_block = block->prev;

        Z_RemoveBlock(block);

        remaining -= block->size;

        if (block->user != nullptr)
        {
            *block->user = nullptr;
        }

        free(block);

        block = next_block;
    }

    return true;
}

//
// Z_Malloc
// You can pass a NULL user if the tag is < PU::PURGELEVEL.
//

auto Z_Malloc(size_t size, PU tag, void *user) -> void *
{
    if (tag < PU::ZERO || tag >= PU::NUM_TAGS || tag == PU::FREE)
    {
        S_Error(fmt::format("Z_Malloc: attempted to allocate a block with an invalid tag: {}", static_cast<int>(tag)));
    }

    if (user == nullptr && tag >= PU::PURGELEVEL)
    {
        S_Error("Z_Malloc: an owner is required for purgable blocks");
    }

    // Malloc a block of the required size
    memblock_s *newblock = nullptr;

    while (newblock == nullptr)
    {
        newblock = (memblock_s *)malloc(sizeof(memblock_s) + size);

        if (newblock == nullptr)
        {
            if (!ClearCache(sizeof(memblock_s) + size))
            {
                I_Error("Z_Malloc: failed on allocation of %i bytes", size);
            }
        }
    }

    newblock->tag = tag;

    // Hook into the linked list for this tag type

    newblock->id   = ZONEID;
    newblock->user = &user;
    newblock->size = size;

    Z_InsertBlock(newblock);

    auto *data   = reinterpret_cast<unsigned char *>(newblock);
    void *result = data + sizeof(memblock_s);

    if (user != nullptr)
    {
        *newblock->user = result;
    }

    return result;
}


//
// Z_FreeTags
//

void Z_FreeTags(int lowtag, int hightag)
{
    int i;

    for (i = lowtag; i <= hightag; ++i)
    {
        memblock_s *block;
        memblock_s *next;

        // Free all in this chain

        for (block = allocated_blocks[i]; block != NULL;)
        {
            next = block->next;

            // Free this block

            if (block->user != NULL)
            {
                *block->user = NULL;
            }

            free(block);

            // Jump to the next in the chain

            block = next;
        }

        // This chain is empty now

        allocated_blocks[i] = NULL;
    }
}


//
// Z_DumpHeap
//
void Z_DumpHeap(int lowtag [[maybe_unused]], int hightag [[maybe_unused]])
{
    // broken

#if 0
    memblock_s*	block;

    printf ("zone size: %i  location: %p\n",
	    mainzone->size,mainzone);

    printf ("tag range: %i to %i\n",
	    lowtag, hightag);

    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	if (block->tag >= lowtag && block->tag <= hightag)
	    printf ("block:%p    size:%7i    user:%p    tag:%3i\n",
		    block, block->size, block->user, block->tag);

	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}

	if ( (byte *)block + block->size != (byte *)block->next)
	    printf ("ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	    printf ("ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU::FREE && block->next->tag == PU::FREE)
	    printf ("ERROR: two consecutive free blocks\n");
    }
#endif
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap(FILE *f [[maybe_unused]])
{
    // broken
#if 0
    memblock_s*	block;

    fprintf (f,"zone size: %i  location: %p\n",mainzone->size,mainzone);

    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	fprintf (f,"block:%p    size:%7i    user:%p    tag:%3i\n",
		 block, block->size, block->user, block->tag);

	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}

	if ( (byte *)block + block->size != (byte *)block->next)
	    fprintf (f,"ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	    fprintf (f,"ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU::FREE && block->next->tag == PU::FREE)
	    fprintf (f,"ERROR: two consecutive free blocks\n");
    }
#endif
}


//
// Z_CheckHeap
//
void Z_CheckHeap(void)
{
    memblock_s *block;
    memblock_s *prev;

    // Check all chains
    for (int i = 0; i < static_cast<int>(PU::NUM_TAGS); ++i)
    {
        prev = nullptr;

        for (block = allocated_blocks[i]; block != NULL; block = block->next)
        {
            if (block->id != ZONEID)
            {
                S_Error("Z_CheckHeap: Block without a ZONEID!");
            }

            if (block->prev != prev)
            {
                S_Error("Z_CheckHeap: Doubly-linked list corrupted!");
            }

            prev = block;
        }
    }
}


//
// Z_ChangeTag
//

void Z_ChangeTag2(void *ptr, PU tag, const std::string_view file, int line)
{
    auto *block = reinterpret_cast<memblock_s *>(reinterpret_cast<byte *>(ptr) - sizeof(memblock_s));

    if (block->id != ZONEID)
    {
        S_Error(fmt::format("{}:{}: Z_ChangeTag: block without a ZONEID!", file, line));
    }

    if (tag >= PU::PURGELEVEL && block->user == nullptr)
    {
        S_Error(fmt::format("{}:{}: Z_ChangeTag: an owner is required for purgable blocks", file, line));
    }

    // Remove the block from its current list, and rehook it into
    // its new list.

    Z_RemoveBlock(block);
    block->tag = tag;
    Z_InsertBlock(block);
}

void Z_ChangeUser(void *ptr, void **user)
{
    auto *block = reinterpret_cast<memblock_s *>(reinterpret_cast<byte *>(ptr) - sizeof(memblock_s));

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
    // Limited by the system??

    return -1;
}

unsigned int Z_ZoneSize(void)
{
    return 0;
}

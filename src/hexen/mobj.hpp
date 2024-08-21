#ifndef __MOBJ_HPP__
#define __MOBJ_HPP__

import m_fixed; //#include "m_fixed.hpp"
#include "../tables.hpp" // angle_t
#include "../../opl/opl3.hpp" // intptr_tcd bui c

#include "info.hpp" // spritenum_t

struct subsector_t;

// think_t is a function pointer to a routine to handle an actor
typedef void (*think_t) ();

struct thinker_t
{
    struct thinker_t *prev, *next;
    think_t function;
};

// each sector has a degenmobj_t in it's center for sound origin purposes
struct degenmobj_t
{
    thinker_t thinker;          // not used for anything
    fixed_t x, y, z;
};

union specialval_t
{
    intptr_t i;
    struct mobj_t *m;
    struct player_t *p;
};


struct mobj_t
{
    thinker_t thinker;          // thinker node

    // info for drawing
    fixed_t x, y, z;
    mobj_t *snext, *sprev;       // links in sector (if needed)
    angle_t angle;
    spritenum_t sprite;         // used to find patch_t and flip value
    int frame;                  // might be ord with FF_FULLBRIGHT

    // interaction info
    mobj_t *bnext, *bprev;      // links in blocks (if needed)
    subsector_t *subsector;
    fixed_t floorz, ceilingz;   // closest together of contacted secs
    fixed_t floorpic;           // contacted sec floorpic
    fixed_t radius, height;     // for movement checking
    fixed_t momx, momy, momz;   // momentums
    int validcount;             // if == validcount, already checked
    mobjtype_t type;
    mobjinfo_t *info;           // &mobjinfo[mobj->type]
    int tics;                   // state tic counter
    state_t *state;
    int damage;                 // For missiles
    int flags;
    int flags2;                 // Heretic flags
    specialval_t special1;      // Special info
    specialval_t special2;      // Special info
    int health;
    int movedir;                // 0-7
    int movecount;              // when 0, select a new dir
    mobj_t *target;             // thing being chased/attacked (or NULL)
    // also the originator for missiles
    int reactiontime;           // if non 0, don't attack yet
    // used by player to freeze a bit after
    // teleporting
    int threshold;              // if > 0, the target will be chased
    // no matter what (even if shot)
    player_t *player;           // only valid if type == MT_PLAYER
    int lastlook;               // player number last looked for
    fixed_t floorclip;          // value to use for floor clipping
    int archiveNum;             // Identity during archive
    short tid;                  // thing identifier
    uint8_t special;               // special
    uint8_t args[5];               // special arguments
};

#endif // __MOBJ_HPP__

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
//	Enemy thinking, AI.
//	Action Pointer Functions
//	that are associated with states/frames.
//
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "m_random.hpp"
#include "i_system.hpp"

#include "doomdef.hpp"
#include "p_local.hpp"

#include "s_sound.hpp"

#include "g_game.hpp"

// State.
#include "doomstat.hpp"
#include "r_state.hpp"

// Data.
#include "sounds.hpp"


enum dirtype_t
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
};


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] = {
    DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
    DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] = {
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};


void A_Fall(mobj_t *actor);
//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

mobj_t *soundtarget;

void P_RecursiveSound(sector_t *sec,
    int                         soundblocks)
{
    int i;

    // wake up all monsters in this sector
    if (sec->validcount == validcount
        && sec->soundtraversed <= soundblocks + 1)
    {
        return; // already flooded
    }

    sec->validcount     = validcount;
    sec->soundtraversed = soundblocks + 1;
    sec->soundtarget    = soundtarget;

    for (i = 0; i < sec->linecount; i++)
    {
        line_s *check = sec->lines[i];
        if (!(check->flags & ML_TWOSIDED))
            continue;

        P_LineOpening(check);

        if (openrange <= 0)
            continue; // closed door

        sector_t *other;
        if (sides[check->sidenum[0]].sector == sec)
            other = sides[check->sidenum[1]].sector;
        else
            other = sides[check->sidenum[0]].sector;

        if (check->flags & ML_SOUNDBLOCK)
        {
            if (!soundblocks)
                P_RecursiveSound(other, 1);
        }
        else
            P_RecursiveSound(other, soundblocks);
    }
}


//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target,
    mobj_t *              emmiter)
{
    // [crispy] monsters are deaf with NOTARGET cheat
    if (target && target->player && (target->player->cheats & CF_NOTARGET))
        return;

    soundtarget = target;
    validcount++;
    P_RecursiveSound(emmiter->subsector->sector, 0);
}


//
// P_CheckMeleeRange
//
bool P_CheckMeleeRange(mobj_t *actor)
{
    if (!actor->target)
    {
        return false;
    }

    mobj_t *pl   = actor->target;
    fixed_t dist = P_AproxDistance(pl->x - actor->x, pl->y - actor->y);

    fixed_t range;
    if (gameversion <= exe_doom_1_2)
        range = MELEERANGE;
    else
        range = MELEERANGE - 20 * FRACUNIT + pl->info->radius;

    if (dist >= range)
    {
        return false;
    }


    if (!P_CheckSight(actor, actor->target))
    {
        return false;
    }

    // [crispy] height check for melee attacks
    if (critical->overunder && pl->player)
    {
        if (pl->z >= actor->z + actor->height || actor->z >= pl->z + pl->height)
        {
            return false;
        }
    }

    return true;
}

//
// P_CheckMissileRange
//
bool P_CheckMissileRange(mobj_t *actor)
{
    fixed_t dist;

    if (!P_CheckSight(actor, actor->target))
        return false;

    if (actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy,
        // so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if (actor->reactiontime)
        return false; // do not attack yet

    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance(actor->x - actor->target->x,
               actor->y - actor->target->y)
           - 64 * FRACUNIT;

    if (!actor->info->meleestate)
        dist -= 128 * FRACUNIT; // no melee attack, so fire more

    dist >>= FRACBITS;

    if (actor->type == MT_VILE)
    {
        if (dist > 14 * 64)
            return false; // too far away
    }


    if (actor->type == MT_UNDEAD)
    {
        if (dist < 196)
            return false; // close for fist attack
        dist >>= 1;
    }


    if (actor->type == MT_CYBORG
        || actor->type == MT_SPIDER
        || actor->type == MT_SKULL)
    {
        dist >>= 1;
    }

    if (dist > 200)
        dist = 200;

    if (actor->type == MT_CYBORG && dist > 160)
        dist = 160;

    if (P_Random() < dist)
        return false;

    return true;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
fixed_t xspeed[8] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
fixed_t yspeed[8] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

bool P_Move(mobj_t *actor)
{
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    if (actor->movedir == DI_NODIR)
    {
        return false;
    }

    if ((unsigned)actor->movedir >= 8)
    {
        I_Error("Weird actor->movedir!");
    }

    fixed_t try_x = actor->x + actor->info->speed * xspeed[actor->movedir];
    fixed_t try_y = actor->y + actor->info->speed * yspeed[actor->movedir];

    bool try_ok = P_TryMove(actor, try_x, try_y);

    if (!try_ok)
    {
        // open any specials
        if (actor->flags & MF_FLOAT && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
                actor->z += FLOATSPEED;
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
        {
            return false;
        }

        actor->movedir = DI_NODIR;
        bool good      = false;
        while (numspechit--)
        {
            line_s *ld = spechit[numspechit];
            // if the special is not a door
            // that can be opened,
            // return false
            if (P_UseSpecialLine(actor, ld, 0))
            {
                good = true;
            }
        }
        return good;
    }
    else
    {
        actor->flags &= ~MF_INFLOAT;
    }


    if (!(actor->flags & MF_FLOAT))
        actor->z = actor->floorz;
    return true;
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
bool P_TryWalk(mobj_t *actor)
{
    if (!P_Move(actor))
    {
        return false;
    }

    actor->movecount = P_Random() & 15;
    return true;
}


void P_NewChaseDir(mobj_t *actor)
{
    dirtype_t d[3];

    dirtype_t turnaround;

    if (!actor->target)
        I_Error("P_NewChaseDir: called with no target");

    dirtype_t olddir = static_cast<dirtype_t>(actor->movedir);
    turnaround       = opposite[olddir];

    fixed_t delta_x = actor->target->x - actor->x;
    fixed_t delta_y = actor->target->y - actor->y;

    if (delta_x > 10 * FRACUNIT)
    {
        d[1] = DI_EAST;
    }
    else if (delta_x < -10 * FRACUNIT)
    {
        d[1] = DI_WEST;
    }
    else
    {
        d[1] = DI_NODIR;
    }

    if (delta_y < -10 * FRACUNIT)
    {
        d[2] = DI_SOUTH;
    }
    else if (delta_y > 10 * FRACUNIT)
    {
        d[2] = DI_NORTH;
    }
    else
    {
        d[2] = DI_NODIR;
    }

    // try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((delta_y < 0) << 1) + (delta_x > 0)];
        if (actor->movedir != (int)turnaround && P_TryWalk(actor))
            return;
    }

    // try other directions
    if (P_Random() > 200 || abs(delta_y) > abs(delta_x))
    {
        auto tdir = d[1];
        d[1]      = d[2];
        d[2]      = tdir;
    }

    if (d[1] == turnaround)
    {
        d[1] = DI_NODIR;
    }
    if (d[2] == turnaround)
    {
        d[2] = DI_NODIR;
    }

    if (d[1] != DI_NODIR)
    {
        actor->movedir = d[1];
        if (P_TryWalk(actor))
        {
            // either moved forward or attacked
            return;
        }
    }

    if (d[2] != DI_NODIR)
    {
        actor->movedir = d[2];

        if (P_TryWalk(actor))
        {
            return;
        }
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir != DI_NODIR)
    {
        actor->movedir = olddir;

        if (P_TryWalk(actor))
        {
            return;
        }
    }

    // randomly determine direction of search
    if (P_Random() & 1)
    {
        for (int tdir = DI_EAST;
             tdir <= DI_SOUTHEAST;
             tdir++)
        {
            if (tdir != (int)turnaround)
            {
                actor->movedir = tdir;

                if (P_TryWalk(actor))
                    return;
            }
        }
    }
    else
    {
        for (int tdir = DI_SOUTHEAST;
             tdir != (DI_EAST - 1);
             tdir--)
        {
            if (tdir != (int)turnaround)
            {
                actor->movedir = tdir;

                if (P_TryWalk(actor))
                    return;
            }
        }
    }

    if (turnaround != DI_NODIR)
    {
        actor->movedir = turnaround;
        if (P_TryWalk(actor))
            return;
    }

    actor->movedir = DI_NODIR; // can not move
}


//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
bool P_LookForPlayers(mobj_t *actor,
    bool                      allaround)
{
    int       c;
    int       stop;
    player_t *player;
    angle_t   an;
    fixed_t   dist;

    c    = 0;
    stop = (actor->lastlook - 1) & 3;

    for (;; actor->lastlook = (actor->lastlook + 1) & 3)
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2
            || actor->lastlook == stop)
        {
            // done looking
            return false;
        }

        player = &players[actor->lastlook];

        // [crispy] monsters don't look for players with NOTARGET cheat
        if (player->cheats & CF_NOTARGET)
            continue;

        if (player->health <= 0)
            continue; // dead

        if (!P_CheckSight(actor, player->mo))
            continue; // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x,
                     actor->y,
                     player->mo->x,
                     player->mo->y)
                 - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance(player->mo->x - actor->x,
                    player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > MELEERANGE)
                    continue; // behind back
            }
        }

        actor->target = player->mo;
        return true;
    }

    return false;
}


//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t *mo)
{
    thinker_s *th;
    line_s     junk;

    A_Fall(mo);

    // scan the remaining thinkers
    // to see if all Keens are dead
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mobj_t *mo2 = (mobj_t *)th;
        if (mo2 != mo
            && mo2->type == mo->type
            && mo2->health > 0)
        {
            // other Keen not dead
            return;
        }
    }

    junk.tag = 666;
    EV_DoDoor(&junk, vld_open);
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look(mobj_t *actor)
{
    mobj_t *targ;

    actor->threshold = 0; // any shot will wake up
    targ             = actor->subsector->sector->soundtarget;

    // [crispy] monsters don't look for players with NOTARGET cheat
    if (targ && targ->player && (targ->player->cheats & CF_NOTARGET))
        return;

    if (targ
        && (targ->flags & MF_SHOOTABLE))
    {
        actor->target = targ;

        if (actor->flags & MF_AMBUSH)
        {
            if (P_CheckSight(actor, actor->target))
                goto seeyou;
        }
        else
            goto seeyou;
    }


    if (!P_LookForPlayers(actor, false))
        return;

    // go into chase state
seeyou:
    if (actor->info->seesound)
    {
        int sound;

        switch (actor->info->seesound)
        {
        case sfx_posit1:
        case sfx_posit2:
        case sfx_posit3:
            sound = sfx_posit1 + P_Random() % 3;
            break;

        case sfx_bgsit1:
        case sfx_bgsit2:
            sound = sfx_bgsit1 + P_Random() % 2;
            break;

        default:
            sound = actor->info->seesound;
            break;
        }

        if (actor->type == MT_SPIDER
            || actor->type == MT_CYBORG)
        {
            // full volume
            // [crispy] prevent from adding up volume
            crispy->soundfull ? S_StartSoundOnce(NULL, sound) : S_StartSound(NULL, sound);
        }
        else
            S_StartSound(actor, sound);

        // [crispy] make seesounds uninterruptible
        if (crispy->soundfull)
        {
            S_UnlinkSound(actor);
        }
    }

    P_SetMobjState(actor, static_cast<statenum_t>(actor->info->seestate));
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase(mobj_t *actor)
{
    int delta;

    if (actor->reactiontime)
        actor->reactiontime--;


    // modify target threshold
    if (actor->threshold)
    {
        if (gameversion > exe_doom_1_2 && (!actor->target || actor->target->health <= 0))
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7 << 29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90 / 2;
        else if (delta < 0)
            actor->angle += ANG90 / 2;
    }

    if (!actor->target
        || !(actor->target->flags & MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor, true))
            return; // got a new target

        P_SetMobjState(actor, static_cast<statenum_t>(actor->info->spawnstate));
        return;
    }

    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (gameskill != sk_nightmare && !fastparm)
            P_NewChaseDir(actor);
        return;
    }

    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange(actor))
    {
        if (actor->info->attacksound)
            S_StartSound(actor, actor->info->attacksound);

        P_SetMobjState(actor, static_cast<statenum_t>(actor->info->meleestate));
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (gameskill < sk_nightmare
            && !fastparm && actor->movecount)
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange(actor))
            goto nomissile;

        P_SetMobjState(actor,
            static_cast<statenum_t>(actor->info->missilestate));
        actor->flags |= MF_JUSTATTACKED;
        return;
    }

    // ?
nomissile:
    // possibly choose another target
    if (netgame
        && !actor->threshold
        && !P_CheckSight(actor, actor->target))
    {
        if (P_LookForPlayers(actor, true))
            return; // got a new target
    }

    // chase towards player
    if (--actor->movecount < 0
        || !P_Move(actor))
    {
        P_NewChaseDir(actor);
    }

    // make active sound
    if (actor->info->activesound
        && P_Random() < 3)
    {
        S_StartSound(actor, actor->info->activesound);
    }
}


//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor)
{
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2(actor->x,
        actor->y,
        actor->target->x,
        actor->target->y);

    if (actor->target->flags & MF_SHADOW)
        actor->angle += P_SubRandom() << 21;
}


//
// A_PosAttack
//
void A_PosAttack(mobj_t *actor)
{
    int angle;
    int damage;
    int slope;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    angle = actor->angle;
    slope = P_AimLineAttack(actor, angle, MISSILERANGE);

    S_StartSound(actor, sfx_pistol);
    angle += static_cast<int64_t>(P_SubRandom() << 20);
    damage = ((P_Random() % 5) + 1) * 3;
    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t *actor)
{
    int i;
    int angle;
    int bangle;
    int damage;
    int slope;

    if (!actor->target)
        return;

    S_StartSound(actor, sfx_shotgn);
    A_FaceTarget(actor);
    bangle = actor->angle;
    slope  = P_AimLineAttack(actor, bangle, MISSILERANGE);

    for (i = 0; i < 3; i++)
    {
        angle  = static_cast<int64_t>(bangle + (P_SubRandom() << 20)) & ANG_MAX;
        damage = ((P_Random() % 5) + 1) * 3;
        P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    S_StartSound(actor, sfx_shotgn);
    A_FaceTarget(actor);
    int bangle = actor->angle;
    int slope  = P_AimLineAttack(actor, bangle, MISSILERANGE);

    int angle  = static_cast<int>(static_cast<int64_t>(bangle) + static_cast<int64_t>(P_SubRandom() << 20));
    int damage = ((P_Random() % 5) + 1) * 3;
    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire(mobj_t *actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget(actor);

    if (P_Random() < 40)
        return;

    if (!actor->target
        || actor->target->health <= 0
        || !P_CheckSight(actor, actor->target))
    {
        P_SetMobjState(actor, static_cast<statenum_t>(actor->info->seestate));
    }
}


void A_SpidRefire(mobj_t *actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget(actor);

    if (P_Random() < 10)
        return;

    if (!actor->target
        || actor->target->health <= 0
        || !P_CheckSight(actor, actor->target))
    {
        P_SetMobjState(actor, static_cast<statenum_t>(actor->info->seestate));
    }
}

void A_BspiAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    // launch a missile
    P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);
}


//
// A_TroopAttack
//
void A_TroopAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        S_StartSound(actor, sfx_claw);
        int damage = (P_Random() % 8 + 1) * 3;
        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);
}


void A_SargAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (gameversion > exe_doom_1_2)
    {
        if (!P_CheckMeleeRange(actor))
            return;
    }

    int damage = ((P_Random() % 10) + 1) * 4;

    if (gameversion <= exe_doom_1_2)
        P_LineAttack(actor, actor->angle, MELEERANGE, 0, damage);
    else
        P_DamageMobj(actor->target, actor, actor, damage);
}

void A_HeadAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        int damage = (P_Random() % 6 + 1) * 10;
        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile(actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, MT_ROCKET);
}


void A_BruisAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    // [crispy] face the enemy
    //  A_FaceTarget (actor);
    if (P_CheckMeleeRange(actor))
    {
        S_StartSound(actor, sfx_claw);
        int damage = (P_Random() % 8 + 1) * 10;
        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);
}


//
// A_SkelMissile
//
void A_SkelMissile(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    actor->z += 16 * FRACUNIT; // so missile spawns higher
    auto *mo = P_SpawnMissile(actor, actor->target, MT_TRACER);
    actor->z -= 16 * FRACUNIT; // back to normal

    mo->x += mo->momX;
    mo->y += mo->momY;
    mo->tracer = actor->target;
}

int TRACEANGLE = 0xc000000;

void A_Tracer(mobj_t *actor)
{
    extern int demostarttic;

    if ((gametic - demostarttic) & 3) // [crispy] fix revenant internal demo bug
        return;

    // spawn a puff of smoke behind the rocket
    P_SpawnPuff(actor->x, actor->y, actor->z);

    mobj_t *th = P_SpawnMobj(actor->x - actor->momX, actor->y - actor->momY, actor->z, MT_SMOKE);

    th->momZ = FRACUNIT;
    th->tics -= P_Random() & 3;
    if (th->tics < 1)
        th->tics = 1;

    // adjust direction
    mobj_t *dest = actor->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
    angle_t exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if (exact != actor->angle)
    {
        if (exact - actor->angle > 0x80000000)
        {
            actor->angle -= TRACEANGLE;
            if (exact - actor->angle < 0x80000000)
                actor->angle = exact;
        }
        else
        {
            actor->angle += TRACEANGLE;
            if (exact - actor->angle > 0x80000000)
                actor->angle = exact;
        }
    }

    exact       = actor->angle >> ANGLETOFINESHIFT;
    actor->momX = FixedMul(actor->info->speed, finecosine[exact]);
    actor->momY = FixedMul(actor->info->speed, finesine[exact]);

    // change slope
    fixed_t dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist         = dist / actor->info->speed;

    if (dist < 1)
        dist = 1;
    fixed_t slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;

    if (slope < actor->momZ)
        actor->momZ -= FRACUNIT / 8;
    else
        actor->momZ += FRACUNIT / 8;
}


void A_SkelWhoosh(mobj_t *actor)
{
    if (!actor->target)
        return;
    A_FaceTarget(actor);
    S_StartSound(actor, sfx_skeswg);
}

void A_SkelFist(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {
        int damage = ((P_Random() % 10) + 1) * 6;
        S_StartSound(actor, sfx_skepch);
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}


//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t *corpsehit;
mobj_t *vileobj;
fixed_t viletryx;
fixed_t viletryy;

bool PIT_VileCheck(mobj_t *thing)
{
    if (!(thing->flags & MF_CORPSE))
        return true; // not a monster

    if (thing->tics != -1)
        return true; // not lying still yet

    if (thing->info->raisestate == S_NULL)
        return true; // monster doesn't have a raise state

    int maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;

    if (abs(thing->x - viletryx) > maxdist
        || abs(thing->y - viletryy) > maxdist)
        return true; // not actually touching

    corpsehit       = thing;
    corpsehit->momX = corpsehit->momY = 0;
    corpsehit->height <<= 2;
    bool check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
    corpsehit->height >>= 2;

    if (!check)
        return true; // doesn't fit here

    return false; // got one, so stop checking
}


//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase(mobj_t *actor)
{
    if (actor->movedir != DI_NODIR)
    {
        // check for corpses to raise
        viletryx = actor->x + actor->info->speed * xspeed[actor->movedir];
        viletryy = actor->y + actor->info->speed * yspeed[actor->movedir];

        int xl = (viletryx - bmaporgx - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        int xh = (viletryx - bmaporgx + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        int yl = (viletryy - bmaporgy - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        int yh = (viletryy - bmaporgy + MAXRADIUS * 2) >> MAPBLOCKSHIFT;

        vileobj = actor;
        for (int bx = xl; bx <= xh; bx++)
        {
            for (int by = yl; by <= yh; by++)
            {
                // Call PIT_VileCheck to check
                // whether object is a corpse
                // that can be raised.
                if (!P_BlockThingsIterator(bx, by, PIT_VileCheck))
                {
                    // got one!
                    mobj_t *temp  = actor->target;
                    actor->target = corpsehit;
                    A_FaceTarget(actor);
                    actor->target = temp;

                    P_SetMobjState(actor, S_VILE_HEAL1);
                    S_StartSound(corpsehit, sfx_slop);
                    mobjinfo_t *info = corpsehit->info;

                    P_SetMobjState(corpsehit,
                        static_cast<statenum_t>(info->raisestate));
                    corpsehit->height <<= 2;
                    corpsehit->flags  = info->flags;
                    corpsehit->health = info->spawnhealth;
                    corpsehit->target = NULL;

                    // [crispy] count resurrected monsters
                    extrakills++;

                    // [crispy] resurrected pools of gore ("ghost monsters") are translucent
                    if (corpsehit->height == 0 && corpsehit->radius == 0)
                    {
                        corpsehit->flags |= MF_TRANSLUCENT;
                        fprintf(stderr, "A_VileChase: Resurrected ghost monster (%d) at (%d/%d)!\n",
                            corpsehit->type, corpsehit->x >> FRACBITS, corpsehit->y >> FRACBITS);
                    }

                    return;
                }
            }
        }
    }

    // Return to normal attack.
    A_Chase(actor);
}


//
// A_VileStart
//
void A_VileStart(mobj_t *actor)
{
    S_StartSound(actor, sfx_vilatk);
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire(mobj_t *actor);

void A_StartFire(mobj_t *actor)
{
    S_StartSound(actor, sfx_flamst);
    A_Fire(actor);
}

void A_FireCrackle(mobj_t *actor)
{
    S_StartSound(actor, sfx_flame);
    A_Fire(actor);
}

void A_Fire(mobj_t *actor)
{
    mobj_t *dest = actor->tracer;
    if (!dest)
        return;

    mobj_t *target = P_SubstNullMobj(actor->target);

    // don't move it if the vile lost sight
    if (!P_CheckSight(target, dest))
        return;

    unsigned an = dest->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition(actor);
    actor->x = dest->x + FixedMul(24 * FRACUNIT, finecosine[an]);
    actor->y = dest->y + FixedMul(24 * FRACUNIT, finesine[an]);
    actor->z = dest->z;
    P_SetThingPosition(actor);
}


//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    mobj_t *fog = P_SpawnMobj(actor->target->x, actor->target->x, actor->target->z, MT_FIRE);

    actor->tracer = fog;
    fog->target   = actor;
    fog->tracer   = actor->target;
    // [crispy] play DSFLAMST sound when Arch-Vile spawns fire attack
    if (crispy->soundfix && I_GetSfxLumpNum(&S_sfx[sfx_flamst]) != -1)
        S_StartSound(fog, sfx_flamst);
    A_Fire(fog);
}


//
// A_VileAttack
//
void A_VileAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (!P_CheckSight(actor, actor->target))
        return;

    S_StartSound(actor, sfx_barexp);
    P_DamageMobj(actor->target, actor, actor, 20);
    actor->target->momZ = 1000 * FRACUNIT / actor->target->info->mass;

    int an = actor->angle >> ANGLETOFINESHIFT;

    mobj_t *fire = actor->tracer;

    if (!fire)
        return;

    // move the fire between the vile and the player
    fire->x = actor->target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
    fire->y = actor->target->y - FixedMul(24 * FRACUNIT, finesine[an]);
    P_RadiusAttack(fire, actor, 70);
}


//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
//#define FATSPREAD (ANG90 / 8)
constexpr auto FATSPREAD = (ANG90 / 8);

void A_FatRaise(mobj_t *actor)
{
    A_FaceTarget(actor);
    S_StartSound(actor, sfx_manatk);
}


void A_FatAttack1(mobj_t *actor)
{
    A_FaceTarget(actor);

    // Change direction  to ...
    actor->angle += FATSPREAD;
    mobj_t *target = P_SubstNullMobj(actor->target);
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mobj_t *mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    int an   = mo->angle >> ANGLETOFINESHIFT;
    mo->momX = FixedMul(mo->info->speed, finecosine[an]);
    mo->momY = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor)
{
    A_FaceTarget(actor);
    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    mobj_t *target = P_SubstNullMobj(actor->target);
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mobj_t *mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD * 2;
    int an   = mo->angle >> ANGLETOFINESHIFT;
    mo->momX = FixedMul(mo->info->speed, finecosine[an]);
    mo->momY = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor)
{
    A_FaceTarget(actor);

    mobj_t *target = P_SubstNullMobj(actor->target);

    mobj_t *mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD / 2;
    int an   = mo->angle >> ANGLETOFINESHIFT;
    mo->momX = FixedMul(mo->info->speed, finecosine[an]);
    mo->momY = FixedMul(mo->info->speed, finesine[an]);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD / 2;
    an       = mo->angle >> ANGLETOFINESHIFT;
    mo->momX = FixedMul(mo->info->speed, finecosine[an]);
    mo->momY = FixedMul(mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED (20 * FRACUNIT)

void A_SkullAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    mobj_t *dest = actor->target;
    actor->flags |= MF_SKULLFLY;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    angle_t an  = actor->angle >> ANGLETOFINESHIFT;
    actor->momX = FixedMul(SKULLSPEED, finecosine[an]);
    actor->momY = FixedMul(SKULLSPEED, finesine[an]);
    int dist    = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist        = dist / SKULLSPEED;

    if (dist < 1)
        dist = 1;
    actor->momZ = (dest->z + (dest->height >> 1) - actor->z) / dist;
}


//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void A_PainShootSkull(mobj_t *actor,
    angle_t                   angle)
{
    // count total number of skull currently on the level
    int count = 0;

    thinker_s *currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        if ((currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
            && ((mobj_t *)currentthinker)->type == MT_SKULL)
            count++;
        currentthinker = currentthinker->next;
    }

    // if there are allready 20 skulls on the level,
    // don't spit another one
    if (count > 20)
        return;


    // okay, there's place for another one
    angle_t an = angle >> ANGLETOFINESHIFT;

    int prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;

    fixed_t x = actor->x + FixedMul(prestep, finecosine[an]);
    fixed_t y = actor->y + FixedMul(prestep, finesine[an]);
    fixed_t z = actor->z + 8 * FRACUNIT;

    mobj_t *newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

    // Check for movements.
    if (!P_TryMove(newmobj, newmobj->x, newmobj->y))
    {
        // kill it immediately
        P_DamageMobj(newmobj, actor, actor, 10000);
        return;
    }

    // [crispy] Lost Souls bleed Puffs
    if (crispy->coloredblood)
        newmobj->flags |= MF_NOBLOOD;

    newmobj->target = actor->target;
    A_SkullAttack(newmobj);
}


//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    A_PainShootSkull(actor, actor->angle);
}


void A_PainDie(mobj_t *actor)
{
    A_Fall(actor);
    A_PainShootSkull(actor, actor->angle + ANG90);
    A_PainShootSkull(actor, actor->angle + ANG180);
    A_PainShootSkull(actor, actor->angle + ANG270);
}


void A_Scream(mobj_t *actor)
{
    int sound;

    switch (actor->info->deathsound)
    {
    case 0:
        return;

    case sfx_podth1:
    case sfx_podth2:
    case sfx_podth3:
        sound = sfx_podth1 + P_Random() % 3;
        break;

    case sfx_bgdth1:
    case sfx_bgdth2:
        sound = sfx_bgdth1 + P_Random() % 2;
        break;

    default:
        sound = actor->info->deathsound;
        break;
    }

    // Check for bosses.
    if (actor->type == MT_SPIDER
        || actor->type == MT_CYBORG)
    {
        // full volume
        // [crispy] prevent from adding up volume
        if (crispy->soundfull)
        {
            S_StartSoundOnce(NULL, sound);
        }
        else
        {
            S_StartSound(NULL, sound);
        };
    }
    else
    {
        S_StartSound(actor, sound);
    }
}


void A_XScream(mobj_t *actor)
{
    S_StartSound(actor, sfx_slop);
}


void A_Pain(mobj_t *actor)
{
    if (actor->info->painsound)
        S_StartSound(actor, actor->info->painsound);
}


void A_Fall(mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode(mobj_t *thingy)
{
    P_RadiusAttack(thingy, thingy->target, 128);
}

// Check whether the death of the specified monster type is allowed
// to trigger the end of episode special action.
//
// This behavior changed in v1.9, the most notable effect of which
// was to break uac_dead.wad

static bool CheckBossEnd(mobjtype_t motype)
{
    if (gameversion < exe_ultimate)
    {
        if (gamemap != 8)
        {
            return false;
        }

        // Baron death on later episodes is nothing special.

        if (motype == MT_BRUISER && gameepisode != 1)
        {
            return false;
        }

        return true;
    }
    else
    {
        // New logic that appeared in Ultimate Doom.
        // Looks like the logic was overhauled while adding in the
        // episode 4 support.  Now bosses only trigger on their
        // specific episode.

        switch (gameepisode)
        {
        case 1:
            return gamemap == 8 && motype == MT_BRUISER;

        case 2:
            return gamemap == 8 && motype == MT_CYBORG;

        case 3:
            return gamemap == 8 && motype == MT_SPIDER;

        case 4:
            return (gamemap == 6 && motype == MT_CYBORG)
                   || (gamemap == 8 && motype == MT_SPIDER);

        // [crispy] Sigil
        case 5:
            return false;

        default:
            return gamemap == 8;
        }
    }
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath(mobj_t *mo)
{
    if (gamemode == GameMode_t::commercial)
    {
        if (gamemap != 7 &&
            // [crispy] Master Levels in PC slot 7
            !(crispy->singleplayer && gamemission == pack_master && (gamemap == 14 || gamemap == 15 || gamemap == 16)))
            return;

        if ((mo->type != MT_FATSO)
            && (mo->type != MT_BABY))
            return;
    }
    else
    {
        if (!CheckBossEnd(mo->type))
        {
            return;
        }
    }

    // make sure there is a player alive for victory
    int i;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i] && players[i].health > 0)
        {
            break;
        }
    }

    if (i == MAXPLAYERS)
        return; // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (thinker_s *th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mobj_t *mo2 = (mobj_t *)th;
        if (mo2 != mo
            && mo2->type == mo->type
            && mo2->health > 0)
        {
            // other boss not dead
            return;
        }
    }

    // Used to replace
    //   line_s junk.tag = 666;
    // with
    //   auto junk = junkTag(666);
    constexpr auto junkTag = [](const short tag) {
        line_s junk;
        junk.tag = tag;
        return junk;
    };


    // victory!
    if (gamemode == GameMode_t::commercial)
    {
        if (gamemap == 7 ||
            // [crispy] Master Levels in PC slot 7
            (crispy->singleplayer && gamemission == pack_master && (gamemap == 14 || gamemap == 15 || gamemap == 16)))
        {
            if (mo->type == MT_FATSO)
            {
                auto junk = junkTag(666);
                EV_DoFloor(&junk, lowerFloorToLowest);
                return;
            }

            if (mo->type == MT_BABY)
            {
                //line_s junk.tag = 667;
                auto junk = junkTag(667);
                EV_DoFloor(&junk, raiseToTexture);
                return;
            }
        }
    }
    else
    {
        auto junk = junkTag(666);
        switch (gameepisode)
        {
        case 1:
            //line_s junk.tag = 666;
            EV_DoFloor(&junk, lowerFloorToLowest);
            return;
            break;

        case 4:
            switch (gamemap)
            {
            case 6:
                //line_s junk.tag = 666;
                EV_DoDoor(&junk, vld_blazeOpen);
                return;
                break;

            case 8:
                //line_s junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                return;
                break;
            }
        }
    }

    G_ExitLevel();
}


void A_Hoof(mobj_t *mo)
{
    S_StartSound(mo, sfx_hoof);
    A_Chase(mo);
}

void A_Metal(mobj_t *mo)
{
    S_StartSound(mo, sfx_metal);
    A_Chase(mo);
}

void A_BabyMetal(mobj_t *mo)
{
    S_StartSound(mo, sfx_bspwlk);
    A_Chase(mo);
}

void A_OpenShotgun2(mobj_t *mobj [[maybe_unused]], player_t *player, pspdef_t *psp [[maybe_unused]])
{
    if (!player) return;                 // [crispy] let pspr action pointers get called from mobj states
    S_StartSound(player->so, sfx_dbopn); // [crispy] weapon sound source
}

void A_LoadShotgun2(mobj_t *mobj [[maybe_unused]], player_t *player, pspdef_t *psp [[maybe_unused]])
{
    if (!player) return;                  // [crispy] let pspr action pointers get called from mobj states
    S_StartSound(player->so, sfx_dbload); // [crispy] weapon sound source
}

void A_ReFire(mobj_t *mobj, player_t *player, pspdef_t *psp);

void A_CloseShotgun2(mobj_t *mobj [[maybe_unused]], player_t *player, pspdef_t *psp)
{
    if (!player) return;                 // [crispy] let pspr action pointers get called from mobj states
    S_StartSound(player->so, sfx_dbcls); // [crispy] weapon sound source
    A_ReFire(NULL, player, psp);         // [crispy] let pspr action pointers get called from mobj states
}


mobj_t **  braintargets    = NULL;
int        numbraintargets = 0; // [crispy] initialize
int        braintargeton   = 0;
static int maxbraintargets; // [crispy] remove braintargets limit

void A_BrainAwake(mobj_t *mo [[maybe_unused]])
{
    // find all the target spots
    numbraintargets = 0;
    braintargeton   = 0;

    thinker_s *thinker = thinkercap.next;
    for (thinker = thinkercap.next;
         thinker != &thinkercap;
         thinker = thinker->next)
    {
        if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
            continue; // not a mobj

        mobj_t *m = (mobj_t *)thinker;

        if (m->type == MT_BOSSTARGET)
        {
            // [crispy] remove braintargets limit
            if (numbraintargets == maxbraintargets)
            {
                maxbraintargets = maxbraintargets ? 2 * maxbraintargets : 32;
                braintargets    = static_cast<decltype(braintargets)>(I_Realloc(braintargets, maxbraintargets * sizeof(*braintargets)));

                if (maxbraintargets > 32)
                    fprintf(stderr, "R_BrainAwake: Raised braintargets limit to %d.\n", maxbraintargets);
            }

            braintargets[numbraintargets] = m;
            numbraintargets++;
        }
    }

    S_StartSound(NULL, sfx_bossit);

    // [crispy] prevent braintarget overflow
    // (e.g. in two subsequent maps featuring a brain spitter)
    if (braintargeton >= numbraintargets)
    {
        braintargeton = 0;
    }

    // [crispy] no spawn spots available
    if (numbraintargets == 0)
        numbraintargets = -1;
}


void A_BrainPain(mobj_t *mo [[maybe_unused]])
{
    // [crispy] prevent from adding up volume
    crispy->soundfull ? S_StartSoundOnce(NULL, sfx_bospn) : S_StartSound(NULL, sfx_bospn);
}


void A_BrainScream(mobj_t *mo)
{
    for (int x = mo->x - 196 * FRACUNIT; x < mo->x + 320 * FRACUNIT; x += FRACUNIT * 8)
    {
        int     y  = mo->y - 320 * FRACUNIT;
        int     z  = 128 + P_Random() * 2 * FRACUNIT;
        mobj_t *th = P_SpawnMobj(x, y, z, MT_ROCKET);
        th->momZ   = P_Random() * 512;

        P_SetMobjState(th, S_BRAINEXPLODE1);

        th->tics = std::max(th->tics - (P_Random() & 7), 1);
    }

    S_StartSound(NULL, sfx_bosdth);
}


void A_BrainExplode(mobj_t *mo)
{
    auto    x  = mo->x + P_SubRandom() * 2048;
    auto    y  = mo->y;
    auto    z  = 128 + P_Random() * 2 * FRACUNIT;
    mobj_t *th = P_SpawnMobj(x, y, z, MT_ROCKET);
    th->momZ   = P_Random() * 512;

    P_SetMobjState(th, S_BRAINEXPLODE1);

    // th->tics -= P_Random() & 7;
    // if (th->tics < 1)
    //     th->tics = 1;
    th->tics = std::max(th->tics - (P_Random() & 7), 1);

    // [crispy] brain explosions are translucent
    th->flags |= MF_TRANSLUCENT;
}


void A_BrainDie(mobj_t *mo [[maybe_unused]])
{
    G_ExitLevel();
}

void A_BrainSpit(mobj_t *mo)
{
    static int easy = 0;

    easy ^= 1;
    if (gameskill <= sk_easy && (!easy))
        return;

    // [crispy] avoid division by zero by recalculating the number of spawn spots
    if (numbraintargets == 0)
    {
        A_BrainAwake(NULL);
    }

    // [crispy] still no spawn spots available
    if (numbraintargets == -1)
    {
        return;
    }

    // shoot a cube at current target
    mobj_t *targ = braintargets[braintargeton];
    if (numbraintargets == 0)
    {
        I_Error("A_BrainSpit: numbraintargets was 0 (vanilla crashes here)");
    }
    braintargeton = (braintargeton + 1) % numbraintargets;

    // spawn brain missile
    mobj_t *newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);
    newmobj->target = targ;
    newmobj->reactiontime =
        ((targ->y - mo->y) / newmobj->momY) / newmobj->state->tics;

    S_StartSound(nullptr, sfx_bospit);
}


void A_SpawnFly(mobj_t *mo);

// travelling cube sound
void A_SpawnSound(mobj_t *mo)
{
    S_StartSound(mo, sfx_boscub);
    A_SpawnFly(mo);
}

void A_SpawnFly(mobj_t *mo)
{
    if (--mo->reactiontime)
        return; // still flying

    mobj_t *targ = P_SubstNullMobj(mo->target);

    // First spawn teleport fog.
    mobj_t *fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);
    S_StartSound(fog, sfx_telept);

    // Randomly select monster to spawn.
    // Probability distribution (kind of :),
    // decreasing likelihood.
    mobjtype_t type;
    if (auto r = P_Random(); r < 50)
        type = MT_TROOP;
    else if (r < 90)
        type = MT_SERGEANT;
    else if (r < 120)
        type = MT_SHADOWS;
    else if (r < 130)
        type = MT_PAIN;
    else if (r < 160)
        type = MT_HEAD;
    else if (r < 162)
        type = MT_VILE;
    else if (r < 172)
        type = MT_UNDEAD;
    else if (r < 192)
        type = MT_BABY;
    else if (r < 222)
        type = MT_FATSO;
    else if (r < 246)
        type = MT_KNIGHT;
    else
        type = MT_BRUISER;

    mobj_t *newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);

    // [crispy] count spawned monsters
    extrakills++;

    if (P_LookForPlayers(newmobj, true))
        P_SetMobjState(newmobj, static_cast<statenum_t>(newmobj->info->seestate));

    // telefrag anything in this spot
    P_TeleportMove(newmobj, newmobj->x, newmobj->y);

    // remove self (i.e., cube).
    P_RemoveMobj(mo);
}


void A_PlayerScream(mobj_t *mo)
{
    // Default death sound.
    auto sound = sfx_pldeth;

    if ((gamemode == GameMode_t::commercial)
        && (mo->health < -50))
    {
        // IF THE PLAYER DIES
        // LESS THAN -50% WITHOUT GIBBING
        sound = sfx_pdiehi;
    }

    S_StartSound(mo, sound);
}

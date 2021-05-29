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
// DESCRIPTION: Door animation code (opening/closing)
//

//#include <stdio.h> // for NULL, fprintf, stderr
#include <cstdio> // for NULL, fprintf, stderr

#include "../../utils/memory.hpp" // for zmalloc
#include "../common.hpp"          // for fmt::print
#include "../crispy.hpp"          // for crispy, crispy_t
#include "../deh_str.hpp"         // for DEH_String
#include "../i_timer.hpp"         // for TICRATE
#include "../m_fixed.hpp"         // for FRACUNIT
#include "../z_zone.hpp"          // for PU, PU::LEVSPEC
#include "d_englsh.hpp"           // for PD_BLUEK, PD_BLUEO, PD_REDK, PD_REDO
#include "d_player.hpp"           // for player_t
#include "d_think.hpp"            // for actionf_p1, think_t, thinker_s
#include "doomdata.hpp"           // for NO_INDEX
#include "doomdef.hpp"            // for it_bluecard, it_redcard, it_yellow...
#include "p_local.hpp"            // for P_AddThinker, P_RemoveThinker, KEY...
#include "p_mobj.hpp"             // for mobj_t
#include "p_spec.hpp"             // for vldoor_t, P_FindLowestCeilingSurro...
#include "r_defs.hpp"             // for sector_t, line_s, side_t
#include "r_state.hpp"            // for sectors, sides
#include "s_sound.hpp"            // for S_StartSound
#include "sounds.hpp"             // for sfx_doropn, sfx_oof, sfx_bdcls


#if 0
//
// Sliding door frame information
//
slidename_t	slideFrameNames[MAXSLIDEDOORS] =
{
    {"GDOORF1","GDOORF2","GDOORF3","GDOORF4",	// front
     "GDOORB1","GDOORB2","GDOORB3","GDOORB4"},	// back

    {"\0","\0","\0","\0"}
};
#endif


//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void T_VerticalDoor(vldoor_t *door)
{
    switch (door->direction)
    {
    case 0: {
        // WAITING
        if (!static_cast<bool>(--door->topcountdown))
        {
            switch (door->type)
            {
            case vld_blazeRaise:
                door->direction = -1; // time to go back down
                S_StartSound(&door->sector->soundorg, sfx_bdcls);
                break;

            case vld_normal:
                door->direction = -1; // time to go back down
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
                break;

            case vld_close30ThenOpen:
                door->direction = 1;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;

            default:
                break;
            }
        }
        break;
    }

    case 2: { //  INITIAL WAIT
        if (!static_cast<bool>(--door->topcountdown))
        {
            switch (door->type)
            {
            case vld_raiseIn5Mins:
                door->direction = 1;
                door->type      = vld_normal;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;

            default:
                break;
            }
        }
        break;
    }

    case -1: {
        // DOWN
        auto res = T_MovePlane(door->sector,
            door->speed,
            door->sector->floorheight,
            false, 1, door->direction);
        if (res == result_e::pastdest)
        {
            switch (door->type)
            {
            case vld_blazeRaise:
            case vld_blazeClose:
                door->sector->specialdata = nullptr;
                P_RemoveThinker(&door->thinker); // unlink and free
                // [crispy] fix "fast doors make two closing sounds"
                if (!static_cast<bool>(crispy->soundfix))
                {
                    S_StartSound(&door->sector->soundorg, sfx_bdcls);
                }
                break;

            case vld_normal:
            case vld_close:
                door->sector->specialdata = nullptr;
                P_RemoveThinker(&door->thinker); // unlink and free
                break;

            case vld_close30ThenOpen:
                door->direction    = 0;
                door->topcountdown = TICRATE * 30;
                break;

            default:
                break;
            }
        }
        else if (res == result_e::crushed)
        {
            switch (door->type)
            {
            case vld_blazeClose:
            case vld_close: // DO NOT GO BACK UP!
                break;

            // [crispy] fix "fast doors reopening with wrong sound"
            case vld_blazeRaise:
                if (static_cast<bool>(crispy->soundfix))
                {
                    door->direction = 1;
                    S_StartSound(&door->sector->soundorg, sfx_bdopn);
                    break;
                };
                [[fallthrough]];

            default:
                door->direction = 1;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;
            }
        }
        break;
    }
    case 1: {
        // UP
        auto res = T_MovePlane(door->sector,
            door->speed,
            door->topheight,
            false, 1, door->direction);

        if (res == result_e::pastdest)
        {
            switch (door->type)
            {
            case vld_blazeRaise:
            case vld_normal:
                door->direction    = 0; // wait at top
                door->topcountdown = door->topwait;
                break;

            case vld_close30ThenOpen:
            case vld_blazeOpen:
            case vld_open:
                door->sector->specialdata = nullptr;
                P_RemoveThinker(&door->thinker); // unlink and free
                break;

            default:
                break;
            }
        }
        break;
    }
    }
}


//
// EV_DoLockedDoor
// Move a locked door up/down
//

auto EV_DoLockedDoor(line_s *line,
    vldoor_e                 type,
    mobj_t *                 thing) -> int
{
    player_t *p = thing->player;

    if (!static_cast<bool>(p))
    {
        return 0;
    }

    constexpr short BlueLock1   = 99;
    constexpr short BlueLock2   = 133;
    constexpr short RedLock1    = 134;
    constexpr short RedLock2    = 135;
    constexpr short YellowLock1 = 136;
    constexpr short YellowLock2 = 137;

    switch (line->special)
    {
    case BlueLock1: // Blue Lock
    case BlueLock2:
        if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
        {
            p->message = DEH_String(PD_BLUEO);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? p->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_bluecard] = KEYBLINKTICS;
            return 0;
        }
        break;

    case RedLock1: // Red Lock
    case RedLock2:
        if (!p->cards[it_redcard] && !p->cards[it_redskull])
        {
            p->message = DEH_String(PD_REDO);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? p->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_redcard] = KEYBLINKTICS;
            return 0;
        }
        break;

    case YellowLock1: // Yellow Lock
    case YellowLock2:
        if (!p->cards[it_yellowcard] && !p->cards[it_yellowskull])
        {
            p->message = DEH_String(PD_YELLOWO);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? p->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_yellowcard] = KEYBLINKTICS;
            return 0;
        }
        break;
    }

    return EV_DoDoor(line, type);
}


auto EV_DoDoor(line_s *line,
    vldoor_e           type) -> int
{
    int rtn    = 0;
    int secnum = -1;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t *sec = &sectors[secnum];
        if (static_cast<bool>(sec->specialdata))
        {
            continue;
        }


        // new door thinker
        rtn            = 1;
        vldoor_t *door = zmalloc<decltype(door)>(sizeof(*door), PU::LEVSPEC, nullptr);
        P_AddThinker(&door->thinker);
        sec->specialdata = door;

        door->thinker.function.acp1 = reinterpret_cast<actionf_p1>(T_VerticalDoor);
        door->sector                = sec;
        door->type                  = type;
        door->topwait               = VDOORWAIT;
        door->speed                 = VDOORSPEED;

        switch (type)
        {
        case vld_blazeClose:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->direction = -1;
            door->speed     = VDOORSPEED * 4;
            // [crispy] fix door-closing sound playing, even when door is already closed (repeatable walkover trigger)
            if (door->sector->ceilingheight - door->sector->floorheight > 0 || !static_cast<bool>(crispy->soundfix))
            {
                S_StartSound(&door->sector->soundorg, sfx_bdcls);
            }
            break;

        case vld_close:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->direction = -1;
            // [crispy] fix door-closing sound playing, even when door is already closed (repeatable walkover trigger)
            if (door->sector->ceilingheight - door->sector->floorheight > 0 || !static_cast<bool>(crispy->soundfix))
            {
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
            }
            break;

        case vld_close30ThenOpen:
            door->topheight = sec->ceilingheight;
            door->direction = -1;
            // [crispy] fix door-closing sound playing, even when door is already closed (repeatable walkover trigger)
            if (door->sector->ceilingheight - door->sector->floorheight > 0 || !static_cast<bool>(crispy->soundfix))
            {
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
            }
            break;

        case vld_blazeRaise:
        case vld_blazeOpen:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->speed = VDOORSPEED * 4;
            if (door->topheight != sec->ceilingheight)
            {
                S_StartSound(&door->sector->soundorg, sfx_bdopn);
            }
            break;

        case vld_normal:
        case vld_open:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            if (door->topheight != sec->ceilingheight)
            {
                S_StartSound(&door->sector->soundorg, sfx_doropn);
            }
            break;

        default:
            break;
        }
    }
    return rtn;
}


void EV_VerticalDoor_A(line_s *line, player_t *player) //line_s *line, mobj_t *thing)
{
    constexpr short blue_lock_1   = 26;
    constexpr short blue_lock_2   = 32;
    constexpr short yellow_lock_1 = 27;
    constexpr short yellow_lock_2 = 34;
    constexpr short red_lock_1    = 28;
    constexpr short red_lock_2    = 33;

    switch (line->special)
    {
    case blue_lock_1: // Blue Lock
    case blue_lock_2:
        if (!static_cast<bool>(player))
        {
            return;
        }

        if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
        {
            player->message = DEH_String(PD_BLUEK);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? player->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_bluecard] = KEYBLINKTICS;
            return;
        }
        break;

    case yellow_lock_1: // Yellow Lock
    case yellow_lock_2:
        if (!static_cast<bool>(player))
        {
            return;
        }

        if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
        {
            player->message = DEH_String(PD_YELLOWK);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? player->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_yellowcard] = KEYBLINKTICS;
            return;
        }
        break;

    case red_lock_1: // Red Lock
    case red_lock_2:
        if (!static_cast<bool>(player))
        {
            return;
        }

        if (!player->cards[it_redcard] && !player->cards[it_redskull])
        {
            player->message = DEH_String(PD_REDK);
            S_StartSound(static_cast<bool>(crispy->soundfix) ? player->mo : nullptr, sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_redcard] = KEYBLINKTICS;
            return;
        }
        break;
    }
}


auto EV_VerticalDoor_B(sector_t &sec, line_s *line, mobj_t *thing) -> bool
{
    bool break_out = false;
    if (static_cast<bool>(sec.specialdata))
    {
        auto &door = *static_cast<vldoor_t *>(sec.specialdata);
        switch (line->special)
        {
        case 1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
        case 26:
        case 27:
        case 28:
        case 117:
            if (door.direction == -1)
            {
                door.direction = 1; // go back up
                // [crispy] play sound effect when the door is opened again while going down
                if (crispy->soundfix && door.thinker.function.acp1 == (actionf_p1)T_VerticalDoor)
                {
                    S_StartSound(&door.sector->soundorg, line->special == 117 ? sfx_bdopn : sfx_doropn);
                }
            }
            else
            {
                if (!static_cast<bool>(thing->player))
                {
                    return true; // JDC: bad guys never close doors
                }
                // When is a door not a door?
                // In Vanilla, door->direction is set, even though
                // "specialdata" might not actually point at a door.

                if (door.thinker.function.acp1 == (actionf_p1)T_VerticalDoor)
                {
                    door.direction = -1; // start going down immediately
                    // [crispy] play sound effect when the door is closed manually
                    if (static_cast<bool>(crispy->soundfix))
                    {
                        S_StartSound(&door.sector->soundorg, line->special == 117 ? sfx_bdcls : sfx_dorcls);
                    }
                }
                else if (door.thinker.function.acp1 == (actionf_p1)T_PlatRaise)
                {
                    // Erm, this is a plat, not a door.
                    // This notably causes a problem in ep1-0500.lmp where
                    // a plat and a door are cross-referenced; the door
                    // doesn't open on 64-bit.
                    // The direction field in vldoor_t corresponds to the wait
                    // field in plat_t.  Let's set that to -1 instead.
                    auto *plat = reinterpret_cast<plat_t *>(&door);
                    plat->wait = -1;
                }
                else
                {
                    // This isn't a door OR a plat.  Now we're in trouble.
                    fmt::print(stderr, "EV_VerticalDoor: Tried to close something that wasn't a door.\n");

                    // Try closing it anyway. At least it will work on 32-bit
                    // machines.
                    door.direction = -1;
                }
            }
            break_out = true;
            //return;
        }
    }
    return break_out;
}

//
// EV_VerticalDoor : open a door manually, no tag value
void EV_VerticalDoor(line_s *line, mobj_t *thing)
{
    //	Check for locks
    player_t *player = thing->player;

    EV_VerticalDoor_A(line, player);

    // if the sector has an active thinker, use it
    constexpr int side = 1; // only front sides can be used
    if (line->sidenum.at(side) == NO_INDEX)
    {
        // [crispy] do not crash if the wrong side of the door is pushed
        fmt::print(stderr, "EV_VerticalDoor: DR special type on 1-sided linedef\n");
        return;
    }

    sector_t &sec = *sides[line->sidenum.at(side)].sector;

    if (EV_VerticalDoor_B(sec, line, thing)) { return; };

    // for proper sound
    switch (line->special)
    {
    case 117: // BLAZING DOOR RAISE
    case 118: // BLAZING DOOR OPEN
        S_StartSound(&sec.soundorg, sfx_bdopn);
        break;

    case 1: // NORMAL DOOR SOUND
    case 31:
        S_StartSound(&sec.soundorg, sfx_doropn);
        break;

    default: // LOCKED DOOR SOUND
        S_StartSound(&sec.soundorg, sfx_doropn);
        break;
    }


    // new door thinker
    vldoor_t *door = zmalloc<decltype(door)>(sizeof(*door), PU::LEVSPEC, nullptr);
    P_AddThinker(&door->thinker);
    sec.specialdata             = door;
    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector                = &sec;
    door->direction             = 1;
    door->speed                 = VDOORSPEED;
    door->topwait               = VDOORWAIT;

    switch (line->special)
    {
    case 1:
    case 26:
    case 27:
    case 28:
        door->type = vld_normal;
        break;

    case 31:
    case 32:
    case 33:
    case 34:
        door->type    = vld_open;
        line->special = 0;
        break;

    case 117: // blazing door raise
        door->type  = vld_blazeRaise;
        door->speed = VDOORSPEED * 4;
        break;
    case 118: // blazing door open
        door->type    = vld_blazeOpen;
        line->special = 0;
        door->speed   = VDOORSPEED * 4;
        break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(&sec);
    door->topheight -= 4 * FRACUNIT;
}


//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30(sector_t *sec)
{
    vldoor_t *door;

    door = zmalloc<decltype(door)>(sizeof(*door), PU::LEVSPEC, 0);

    P_AddThinker(&door->thinker);

    sec->specialdata = door;
    sec->special     = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector                = sec;
    door->direction             = 0;
    door->type                  = vld_normal;
    door->speed                 = VDOORSPEED;
    door->topcountdown          = 30 * TICRATE;
}

//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum [[maybe_unused]])
{
    vldoor_t *door;

    door = zmalloc<decltype(door)>(sizeof(*door), PU::LEVSPEC, 0);

    P_AddThinker(&door->thinker);

    sec->specialdata = door;
    sec->special     = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector                = sec;
    door->direction             = 2;
    door->type                  = vld_raiseIn5Mins;
    door->speed                 = VDOORSPEED;
    door->topheight             = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait      = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}


// UNUSED
// Separate into p_slidoor.c?

#if 0 // ABANDONED TO THE MISTS OF TIME!!!
//
// EV_SlidingDoor : slide a door horizontally
// (animate midtexture, then set noblocking line)
//


slideframe_t slideFrames[MAXSLIDEDOORS];

void P_InitSlidingDoorFrames(void)
{
    int		i;
    int		f1;
    int		f2;
    int		f3;
    int		f4;

    // DOOM II ONLY...
    if ( gamemode != commercial)
	return;

    for (i = 0;i < MAXSLIDEDOORS; i++)
    {
	if (!slideFrameNames[i].frontFrame1[0])
	    break;

	f1 = R_TextureNumForName(slideFrameNames[i].frontFrame1);
	f2 = R_TextureNumForName(slideFrameNames[i].frontFrame2);
	f3 = R_TextureNumForName(slideFrameNames[i].frontFrame3);
	f4 = R_TextureNumForName(slideFrameNames[i].frontFrame4);

	slideFrames[i].frontFrames[0] = f1;
	slideFrames[i].frontFrames[1] = f2;
	slideFrames[i].frontFrames[2] = f3;
	slideFrames[i].frontFrames[3] = f4;

	f1 = R_TextureNumForName(slideFrameNames[i].backFrame1);
	f2 = R_TextureNumForName(slideFrameNames[i].backFrame2);
	f3 = R_TextureNumForName(slideFrameNames[i].backFrame3);
	f4 = R_TextureNumForName(slideFrameNames[i].backFrame4);

	slideFrames[i].backFrames[0] = f1;
	slideFrames[i].backFrames[1] = f2;
	slideFrames[i].backFrames[2] = f3;
	slideFrames[i].backFrames[3] = f4;
    }
}


//
// Return index into "slideFrames" array
// for which door type to use
//
int P_FindSlidingDoorType(line_s*	line)
{
    int		i;
    int		val;

    for (i = 0;i < MAXSLIDEDOORS;i++)
    {
	val = sides[line->sidenum[0]].midtexture;
	if (val == slideFrames[i].frontFrames[0])
	    return i;
    }

    return -1;
}

void T_SlidingDoor (slidedoor_t*	door)
{
    switch(door->status)
    {
      case sd_opening:
	if (!door->timer--)
	{
	    if (++door->frame == SNUMFRAMES)
	    {
		// IF DOOR IS DONE OPENING...
		sides[door->line->sidenum[0]].midtexture = 0;
		sides[door->line->sidenum[1]].midtexture = 0;
		door->line->flags &= ML_BLOCKING^0xff;

		if (door->type == sdt_openOnly)
		{
		    door->frontsector->specialdata = NULL;
		    P_RemoveThinker (&door->thinker);
		    break;
		}

		door->timer = SDOORWAIT;
		door->status = sd_waiting;
	    }
	    else
	    {
		// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
		door->timer = SWAITTICS;

		sides[door->line->sidenum[0]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    frontFrames[door->frame];
		sides[door->line->sidenum[1]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    backFrames[door->frame];
	    }
	}
	break;

      case sd_waiting:
	// IF DOOR IS DONE WAITING...
	if (!door->timer--)
	{
	    // CAN DOOR CLOSE?
	    if (door->frontsector->thinglist != NULL ||
		door->backsector->thinglist != NULL)
	    {
		door->timer = SDOORWAIT;
		break;
	    }

	    //door->frame = SNUMFRAMES-1;
	    door->status = sd_closing;
	    door->timer = SWAITTICS;
	}
	break;

      case sd_closing:
	if (!door->timer--)
	{
	    if (--door->frame < 0)
	    {
		// IF DOOR IS DONE CLOSING...
		door->line->flags |= ML_BLOCKING;
		door->frontsector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);
		break;
	    }
	    else
	    {
		// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
		door->timer = SWAITTICS;

		sides[door->line->sidenum[0]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    frontFrames[door->frame];
		sides[door->line->sidenum[1]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    backFrames[door->frame];
	    }
	}
	break;
    }
}



void
EV_SlidingDoor
( line_s*	line,
  mobj_t*	thing )
{
    sector_t*		sec;
    slidedoor_t*	door;

    // DOOM II ONLY...
    if (gamemode != commercial)
	return;

    // Make sure door isn't already being animated
    sec = line->frontsector;
    door = NULL;
    if (sec->specialdata)
    {
	if (!thing->player)
	    return;

	door = sec->specialdata;
	if (door->type == sdt_openAndClose)
	{
	    if (door->status == sd_waiting)
		door->status = sd_closing;
	}
	else
	    return;
    }

    // Init sliding door vars
    if (!door)
    {
	door = zmalloc<decltype(door)> (sizeof(*door), PU::LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;

	door->type = sdt_openAndClose;
	door->status = sd_opening;
	door->whichDoorIndex = P_FindSlidingDoorType(line);

	if (door->whichDoorIndex < 0){
	    S_Error("EV_SlidingDoor: Can't use texture for sliding door!");}

	door->frontsector = sec;
	door->backsector = line->backsector;
	door->thinker.function = T_SlidingDoor;
	door->timer = SWAITTICS;
	door->frame = 0;
	door->line = line;
    }
}
#endif

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
// DESCRIPTION:  Ceiling animation (lowering, crushing, raising)
//

#include "../../utils/memory.hpp" // Data.
#include "../z_zone.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp" // State.
#include "p_ceilng.hpp"
#include "p_local.hpp"
#include "r_state.hpp" // State.
#include "s_sound.hpp"
#include "sounds.hpp" // Data.

#include <algorithm>
#include <ranges>
#include <stdexcept>

//
// CEILINGS
//

constexpr fixed_t CEILSPEED   = FRACUNIT;
constexpr size_t  MAXCEILINGS = 30;
//#define CEILWAIT  150


static std::array<ceiling_t *, MAXCEILINGS> activeceilings;

// Returns true if ceiling is in the activeceilings
[[nodiscard]] auto P_CeilingExist(ceiling_t *ceiling) -> bool
{
    auto *const end   = activeceilings.end();
    auto *const found = std::find(activeceilings.begin(), end, ceiling);
    return found != end;
}

auto P_ClearActiveCeilings() -> void
{
    activeceilings.fill(nullptr);
}

//
// T_MoveCeiling
//
void T_MoveCeiling(ceiling_t *ceiling)
{
    switch (ceiling->direction)
    {
    case 0: // IN STASIS
        break;

    case 1: // UP
    {
        auto res = T_MovePlane(ceiling->sector,
            ceiling->speed,
            ceiling->topheight,
            false, 1, ceiling->direction);

        if (!(leveltime & 7))
        {
            if (ceiling->type != ceiling_e::silentCrushAndRaise)
            {
                S_StartSound(&ceiling->sector->soundorg, sfxenum_t::sfx_stnmov);
            }
        }

        if (res == result_e::pastdest)
        {
            switch (ceiling->type)
            {
            case ceiling_e::raiseToHighest: {
                P_RemoveActiveCeiling(ceiling);
            }
            break;

            case ceiling_e::silentCrushAndRaise:
                S_StartSound(&ceiling->sector->soundorg, sfxenum_t::sfx_pstop);
                [[fallthrough]];
            case ceiling_e::fastCrushAndRaise:
            case ceiling_e::crushAndRaise:
                ceiling->direction = -1;
                break;

            default:
                break;
            }
        }
    }
    break;

    case -1: // DOWN
    {
        auto res = T_MovePlane(ceiling->sector,
            ceiling->speed,
            ceiling->bottomheight,
            ceiling->crush, 1, ceiling->direction);

        if (!(leveltime & 7))
        {
            switch (ceiling->type)
            {
            case silentCrushAndRaise: break;
            default:
                S_StartSound(&ceiling->sector->soundorg, sfx_stnmov);
            }
        }

        if (res == pastdest)
        {
            switch (ceiling->type)
            {
            case silentCrushAndRaise:
                S_StartSound(&ceiling->sector->soundorg, sfx_pstop);
                [[fallthrough]];
            case crushAndRaise:
                ceiling->speed = CEILSPEED;
                [[fallthrough]];
            case fastCrushAndRaise:
                ceiling->direction = 1;
                break;

            case lowerAndCrush:
            case lowerToFloor:
                P_RemoveActiveCeiling(ceiling);
                break;

            default:
                break;
            }
        }
        else // ( res != pastdest )
        {
            if (res == crushed)
            {
                switch (ceiling->type)
                {
                case silentCrushAndRaise:
                case crushAndRaise:
                case lowerAndCrush:
                    ceiling->speed = CEILSPEED / 8;
                    break;

                default:
                    break;
                }
            }
        }
    }
    break;
    }
}


//
// EV_DoCeiling
// Move a ceiling up/down and all around!
//
int EV_DoCeiling(line_s *line,
    ceiling_e            type)
{
    int        secnum;
    int        rtn;
    sector_t * sec;
    ceiling_t *ceiling;

    secnum = -1;
    rtn    = 0;

    //	Reactivate in-stasis ceilings...for certain types.
    switch (type)
    {
    case fastCrushAndRaise:
    case silentCrushAndRaise:
    case crushAndRaise:
        P_ActivateInStasisCeiling(line);
    default:
        break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        // new door thinker
        rtn     = 1;
        ceiling = zmalloc<decltype(ceiling)>(sizeof(*ceiling), PU::LEVSPEC, 0);
        P_AddThinker(&ceiling->thinker);
        sec->specialdata               = ceiling;
        ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
        ceiling->sector                = sec;
        ceiling->crush                 = false;

        switch (type)
        {
        case fastCrushAndRaise:
            ceiling->crush        = true;
            ceiling->topheight    = sec->ceilingheight;
            ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
            ceiling->direction    = -1;
            ceiling->speed        = CEILSPEED * 2;
            break;

        case silentCrushAndRaise:
            [[fallthrough]];
        case crushAndRaise:
            ceiling->crush     = true;
            ceiling->topheight = sec->ceilingheight;
            [[fallthrough]];
        case lowerAndCrush:
            [[fallthrough]];
        case lowerToFloor:
            ceiling->bottomheight = sec->floorheight;
            if (type != lowerToFloor)
                ceiling->bottomheight += 8 * FRACUNIT;
            ceiling->direction = -1;
            ceiling->speed     = CEILSPEED;
            break;

        case raiseToHighest:
            ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
            ceiling->direction = 1;
            ceiling->speed     = CEILSPEED;
            break;
        }

        ceiling->tag  = sec->tag;
        ceiling->type = type;
        P_AddActiveCeiling(ceiling);
    }
    return rtn;
}


//
// Add an active ceiling
//
void P_AddActiveCeiling(ceiling_t *c)
{
    auto *const found = std::ranges::find(activeceilings, nullptr);
    if (found == activeceilings.end())
    {
        throw std::length_error("Unable to find empty spot in arrayTest ");
    }
    *found = c;
}


//
// Remove a ceiling's thinker
//
void P_RemoveActiveCeiling(ceiling_t *c)
{
    auto *const found = std::ranges::find(activeceilings, c);
    if (found == activeceilings.end())
    {
        throw std::length_error("Unable to find matching value in P_RemoveActiveCeiling ");
    }
    const auto index = std::distance(activeceilings.begin(), found);

    activeceilings.at(index)->sector->specialdata = nullptr;
    P_RemoveThinker(&activeceilings.at(index)->thinker);
    activeceilings.at(index) = nullptr;
}


//
// Restart a ceiling that's in-stasis
//
void P_ActivateInStasisCeiling(line_s *line)
{
    const auto filter = std::views::filter([line](auto i) {
        //         not nullptr       the tag matches         and the direction is 0?
        return ((i != nullptr) && (i->tag == line->tag) && (i->direction == 0));
    });

    for (auto found : activeceilings | filter)
    {
        found->direction             = found->olddirection;
        found->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
    };
}


//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
int EV_CeilingCrushStop(line_s *line)
{
    int rtn = 0;

    const auto filter = std::views::filter([line](auto i) {
        return ((i != nullptr)           // not nullptr aka theres something there
                && (i->tag == line->tag) // matches tag your looking for
                && (i->direction == 0)); // direction is 0?
    });

    for (auto found : activeceilings | filter)
    {
        found->olddirection         = found->direction;
        found->thinker.function.acv = (actionf_v)NULL;
        found->direction            = 0; // in-stasis
        rtn                         = 1;
    };


    return rtn;
}

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
//	Items: key cards, artifacts, weapon, ammunition.
//

#ifndef DOOM_D_ITEMS_HPP
#define DOOM_D_ITEMS_HPP

#include "doomdef.hpp"


// Weapon info: sprite frames, ammunition use.
struct weaponinfo_t {
    ammotype_t ammo;
    int        upstate;
    int        downstate;
    int        readystate;
    int        atkstate;
    int        flashstate;
};

extern weaponinfo_t weaponinfo[NUMWEAPONS];

#endif // DOOM_D_ITEMS_HPP

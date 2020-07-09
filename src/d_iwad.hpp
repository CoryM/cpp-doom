//
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
//     Find IWAD and initialize according to IWAD type.
//

#pragma once
#ifndef __D_IWAD__
#define __D_IWAD__

#include <string>
#include <string_view>

#include "d_mode.hpp"

#define IWAD_MASK_DOOM ((1 << doom)        \
                        | (1 << doom2)     \
                        | (1 << pack_tnt)  \
                        | (1 << pack_plut) \
                        | (1 << pack_chex) \
                        | (1 << pack_hacx))
#define IWAD_MASK_HERETIC (1 << heretic)
#define IWAD_MASK_HEXEN   (1 << hexen)
#define IWAD_MASK_STRIFE  (1 << strife)

struct iwad_t {
    const char *  name;
    GameMission_t mission;
    GameMode_t    mode;
    const char *  description;
};

[[nodiscard]] std::string    D_FindWADByName(const std::string_view filename);
[[nodiscard]] std::string    D_TryFindWADByName(const std::string_view filename);
[[nodiscard]] std::string    D_FindIWAD(int mask, GameMission_t *mission);
[[nodiscard]] const iwad_t **D_FindAllIWADs(int mask);
[[nodiscard]] const char *   D_SaveGameIWADName(GameMission_t gamemission);
[[nodiscard]] const char *   D_SuggestIWADName(GameMission_t mission, GameMode_t mode);
[[nodiscard]] const char *   D_SuggestGameName(GameMission_t mission, GameMode_t mode);

// Helper function to get Environment Variables into a string_view
[[nodiscard]] std::string env_view(const std::string_view envVar);

void v_iwadDirs_init();
void v_iwadDirs_clear();


#endif

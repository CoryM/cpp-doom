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

#ifndef __D_IWAD__
#define __D_IWAD__

#include <string>
#include <string_view>

#include "d_mode.hpp"

constexpr unsigned int IWAD_MASK_DOOM = ((1 << GameMission_t::doom)
                                         | (1 << GameMission_t::doom2)
                                         | (1 << GameMission_t::pack_tnt)
                                         | (1 << GameMission_t::pack_plut)
                                         | (1 << GameMission_t::pack_chex)
                                         | (1 << GameMission_t::pack_hacx));

constexpr unsigned int IWAD_MASK_HERETIC = (1 << GameMission_t::heretic);
constexpr unsigned int IWAD_MASK_HEXEN   = (1 << GameMission_t::hexen);
constexpr unsigned int IWAD_MASK_STRIFE  = (1 << GameMission_t::strife);

struct iwad_t {
    const char *  name;
    GameMission_t mission;
    GameMode_t    mode;
    const char *  description;
};

[[nodiscard]] auto D_FindWADByName(std::string_view filename) -> std::string;
[[nodiscard]] auto D_TryFindWADByName(std::string_view filename) -> std::string;
[[nodiscard]] auto D_FindIWAD(int mask, GameMission_t *mission) -> std::string;
[[nodiscard]] auto D_FindAllIWADs(int mask) -> const iwad_t **;
[[nodiscard]] auto D_SaveGameIWADName(GameMission_t gamemission) -> const char *;
[[nodiscard]] auto D_SuggestIWADName(GameMission_t mission, GameMode_t mode) -> const char *;
[[nodiscard]] auto D_SuggestGameName(GameMission_t mission, GameMode_t mode) -> const char *;

// Helper function to get Environment Variables into a string_view
[[nodiscard]] auto env_view(std::string_view envVar) -> std::string;

void v_iwadDirs_init();
void v_iwadDirs_clear();


#endif

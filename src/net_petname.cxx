module;
//
// Copyright(C) 2019 Jonathan Dowland
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
//     Generate a randomized, private, memorable name for a Player
//

//#include <cstdlib>
//#include <ctime>
//#include "doomtype.hpp"
//#include "m_misc.hpp"

#include <array>       // std::to_array
#include <random>      // std::random_device, std::mt19937, std::uniform_int_distribution
#include <string_view> // std::string_view

#include "i_system.hpp" // I_Error
import i_exit; //#include "i_error.hpp"

export module net_petname;

constexpr auto adjectives = std::to_array<std::string_view>({
    "Grumpy",
    "Ecstatic",
    "Surly",
    "Prepared",
    "Crafty",
    "Alert",
    "Sluggish",
    "Testy",
    "Reluctant",
    "Languid",
    "Passive",
    "Pacifist",
    "Aggressive",
    "Hostile",
    "Bubbly",
    "Giggly",
    "Laughing",
    "Crying",
    "Frowning",
    "Torpid",
    "Lethargic",
    "Manic",
    "Patient",
    "Protective",
    "Philosophical",
    "Enquiring",
    "Debating",
    "Furious",
    "Laid-Back",
    "Easy-Going",
    "Cromulent",
    "Excitable",
    "Tired",
    "Exhausted",
    "Ruminating",
    "Redundant",
    "Sporty",
    "Ginger",
    "Scary",
    "Posh",
    "Baby",
});

constexpr auto nouns = std::to_array<std::string_view>({
    "Frad",
    // Doom
    "Cacodemon",
    "Arch-Vile",
    "Cyberdemon",
    "Imp",
    "Demon",
    "Mancubus",
    "Arachnotron",
    "Baron",
    "Knight",
    "Revenant",
    // Hexen
    "Ettin",
    "Maulotaur",
    "Centaur",
    "Afrit",
    "Serpent",
    // Heretic
    "Disciple",
    "Gargoyle",
    "Golem",
    "Lich",
    // Strife
    "Sentinel",
    "Acolyte",
    "Templar",
    "Reaver",
    "Spectre",
});

class randbetween {
    private:
      std::random_device rd;  // a seed source for the random number engine
      std::mt19937 gen; // mersenne_twister_engine seeded with rd(); // mersenne_twister_engine seeded with rd()
      std::uniform_int_distribution<> distrib;

    public:
    randbetween() = delete;
    randbetween(int low, int high) : 
      rd(), 
      gen(rd()), 
      distrib(low, high) {
    };
    auto operator()() {
        return distrib(gen);
    };
};

export std::string NET_GetRandomPetName_IMPL()
{
    //InitPetName();
    static auto a_rand = randbetween(0, adjectives.size()-1);
    static auto n_rand = randbetween(0, nouns.size()-1);
    
    const auto a = adjectives.at(a_rand());
    const auto n = nouns.at(n_rand());

    return std::string(a).append(" ").append(n);
}

export char * NET_GetRandomPetName()
{
    const auto petName = NET_GetRandomPetName_IMPL();
    const auto result_len = petName.size() + 1;
    auto result = static_cast<char *>(malloc(result_len));
    if (result) {
        std::copy(petName.begin(), petName.end(), result);
        result[petName.size()] = '\0';
    } else {
        // handle allocation failure
        I_Error("NET_GetRandomPetName: Failed to allocate new string.");
        return NULL;
    }
    return result;
}
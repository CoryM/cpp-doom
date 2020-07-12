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
//     Search for and locate an IWAD file, and initialize according
//     to the IWAD type.
//
#include <vector>
#include "fmt/core.h"

#include "d_iwad.hpp" // Includes common.hpp

#include "../utils/memory.hpp"
#include "deh_str.hpp"
#include "doomkeys.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

static const auto a_iwads = std::to_array<iwad_t>({
    { "doom2.wad",     doom2,     GameMode_t::commercial, "Doom II" },
    { "plutonia.wad",  pack_plut, GameMode_t::commercial, "Final Doom: Plutonia Experiment" },
    { "tnt.wad",       pack_tnt,  GameMode_t::commercial, "Final Doom: TNT: Evilution" },
    { "doom.wad",      doom,      GameMode_t::retail,     "Doom" },
    { "doom1.wad",     doom,      GameMode_t::shareware,  "Doom Shareware" },
    { "chex.wad",      pack_chex, GameMode_t::retail,     "Chex Quest" },
    { "hacx.wad",      pack_hacx, GameMode_t::commercial, "Hacx" },
    { "freedoom2.wad", doom2,     GameMode_t::commercial, "Freedoom: Phase 2" },
    { "freedoom1.wad", doom,      GameMode_t::retail,     "Freedoom: Phase 1" },
    { "freedm.wad",    doom2,     GameMode_t::commercial, "FreeDM" },
    { "heretic.wad",   heretic,   GameMode_t::retail,     "Heretic" },
    { "heretic1.wad",  heretic,   GameMode_t::shareware,  "Heretic Shareware" },
    { "hexen.wad",     hexen,     GameMode_t::commercial, "Hexen" },
    { "strife1.wad",   strife,    GameMode_t::commercial, "Strife" } });

// Helper function to get Environment Variables into a string_view
[[nodiscard]]std::string env_view(const std::string_view envVar)
{
    // returns a NON OWENING String.  The char* is owned and by the OS
    const char * envCharStar = getenv(envVar.data());
    // Creating a string with a nullprt is undefined behaviour.
    if (envCharStar != nullptr)
    {
        return std::string(envCharStar);
    }
    return std::string();
}


// Array of locations to search for IWAD files
//
// DefinedIn: d_iwad.cpp
// UsedIn:    d_iwad.cpp
std::vector<std::string> v_iwadDirs;

void v_iwadDirs_clear()
{
    // if (v_iwadDirs.size())
    // {
    //     std::cout << "Clearing v_iwadDirs size before clear was " << v_iwadDirs.size() << std::endl;
    //     size_t count = 0;
    //     for (const auto &str : v_iwadDirs) {
    //         std::cout << count++ << ") " << str << std::endl;
    //     }
    // }
    v_iwadDirs.clear();
}

void v_iwadDirs_init()
{
    v_iwadDirs = std::vector<std::string>();
    v_iwadDirs_clear();
}


// Returns true if the specified path is a path to a file
// of the specified name.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static auto DirIsFile(const char *path, const char *filename) -> bool
{
    return strchr(path, DIR_SEPARATOR) != nullptr
           && (strcasecmp(M_BaseName(path), filename) == 0);
}


// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static std::string CheckDirectoryHasIWAD(const std::string_view dir, const std::string_view iwadname)
{
    // As a special case, the "directory" may refer directly to an
    // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.

    auto probe = M_FileCaseExists(dir);
    if (DirIsFile(dir.data(), iwadname.data()) && !probe.empty())
    {
        return probe;
    }

    // Construct the full path to the IWAD if it is located in
    // this directory, and check if it exists.
    std::string filename;
    if (!strcmp(dir.data(), "."))
    {
        //filename = M_StringDuplicate(iwadname.data());
        filename = std::string(iwadname);
    }
    else
    {
        //filename = U_StringJoin({dir, DIR_SEPARATOR_S, iwadname});
        filename = std::string(dir) + DIR_SEPARATOR_S + std::string(iwadname);
    }

    probe = M_FileCaseExists(filename);
    if (!probe.empty())
    {
        return probe;
    }

    return std::string();
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise empty string.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static std::string SearchDirectoryForIWAD(const std::string_view dir, int mask, GameMission_t *mission)
{
    for (const auto &i : a_iwads)
    {
        if (((1 << i.mission) & mask) == 0)
        {
            continue;
        }

        auto filename = CheckDirectoryHasIWAD(dir, DEH_String(i.name));

        if (!filename.empty())
        {
            *mission = i.mission;

            return filename;
        }
    }

    return std::string();
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static GameMission_t IdentifyIWADByName(const char *name, int mask)
{
    GameMission_t mission;

    name    = M_BaseName(name);
    mission = none;

    for (const auto &i : a_iwads)
    {
        // Check if the filename is this IWAD name.

        // Only use supported missions:

        if (((1 << i.mission) & mask) == 0)
            continue;

        // Check if it ends in this IWAD name.

        if (!strcasecmp(name, i.name))
        {
            mission = i.mission;
            break;
        }
    }

    return mission;
}

// Add IWAD directories parsed from splitting a path string containing
// paths separated by PATH_SEPARATOR. 'suffix' is a string to concatenate
// to the end of the paths before adding them.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static void AddIWADPath(const std::string_view path, const std::string_view suffix)
{
    // Split into individual dirs within the list.
    auto left = std::string(path);

    for (;;)
    {
        std::size_t pos = left.find(PATH_SEPARATOR);      // position of "live" in str

        if (pos != std::string::npos)
        {
            // Break at the separator and use the left hand side
            // as another iwad dir
            v_iwadDirs.emplace_back(left.substr(0, pos - 1) + std::string(suffix));
            left = left.substr(pos + 1);
        }
        else
        {
            break;
        }
    }

    v_iwadDirs.emplace_back(left + std::string(suffix));

}

// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static void AddXdgDirs(void)
{
    // Quote:
    // > $XDG_DATA_HOME defines the base directory relative to which
    // > user specific data files should be stored. If $XDG_DATA_HOME
    // > is either not set or empty, a default equal to
    // > $HOME/.local/share should be used.

    auto env = env_view("XDG_DATA_HOME");

    if (env.data() == nullptr)
    {
        auto homedir = env_view("HOME");

        if (homedir.data() == nullptr)
        {
            homedir = std::string_view("~/");
        }
        env     = homedir + std::string("/.local/share");
    }

    // We support $XDG_DATA_HOME/games/doom (which will usually be
    // ~/.local/share/games/doom) as a user-writeable extension to
    // the usual /usr/share/games/doom location.
    v_iwadDirs.emplace_back(env + std::string("/games/doom"));

    // Quote:
    // > $XDG_DATA_DIRS defines the preference-ordered set of base
    // > directories to search for data files in addition to the
    // > $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS
    // > should be seperated with a colon ':'.
    // >
    // > If $XDG_DATA_DIRS is either not set or empty, a value equal to
    // > /usr/local/share/:/usr/share/ should be used.
    env = env_view("XDG_DATA_DIRS");
    if (env.data() == nullptr)
    {
        // (Trailing / omitted from paths, as it is added below)
        env = "/usr/local/share:/usr/share";
    }

    // The "standard" location for IWADs on Unix that is supported by most
    // source ports is /usr/share/games/doom - we support this through the
    // XDG_DATA_DIRS mechanism, through which it can be overridden.
    AddIWADPath(env.data(), "/games/doom");

    // The convention set by RBDOOM-3-BFG is to install Doom 3: BFG
    // Edition into this directory, under which includes the Doom
    // Classic WADs.
    AddIWADPath(env.data(), "/games/doom3bfg/base/wads");
}

// Steam on Linux allows installing some select Windows games,
// including the classic Doom series (running DOSBox via Wine).  We
// could parse *.vdf files to more accurately detect installation
// locations, but the defaults are likely to be good enough for just
// about everyone.
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static void AddSteamDirs(void)
{
    auto homedir = env_view("HOME");
    if (homedir.data() == nullptr)
    {
        homedir = std::string_view("/");
    }
    // steampath auto deallocated upon exit
    std::string steampath = homedir +  "/.steam/root/steamapps/common";

    AddIWADPath(steampath, "/Doom 2/base");
    AddIWADPath(steampath, "/Master Levels of Doom/doom2");
    AddIWADPath(steampath, "/Ultimate Doom/base");
    AddIWADPath(steampath, "/Final Doom/base");
    AddIWADPath(steampath, "/DOOM 3 BFG Edition/base/wads");
    AddIWADPath(steampath, "/Heretic Shadow of the Serpent Riders/base");
    AddIWADPath(steampath, "/Hexen/base");
    AddIWADPath(steampath, "/Hexen Deathkings of the Dark Citadel/base");
    AddIWADPath(steampath, "/Strife");
}


// Build a list of IWAD files
//
// UsedIn:    d_iwad.cpp
// DefinedIn: d_iwad.cpp
static void BuildIWADDirList(void)
{
    // Has BuildIWADDirList been run yet (False/No, True/Yes)
    static bool iwad_dirs_built = false;

    if (iwad_dirs_built)
    {
        return;
    }

    // Look in the current directory.  Doom always does this.
    v_iwadDirs.push_back(".");

    // Next check the directory where the executable is located. This might
    // be different from the current directory.
    v_iwadDirs.push_back(M_DirName(myargv[0]));

    // Add DOOMWADDIR if it is in the environment
    char *env = getenv("DOOMWADDIR");
    if (env != nullptr)
    {
        v_iwadDirs.push_back(env);
    }

    // Add dirs from DOOMWADPATH:
    env = getenv("DOOMWADPATH");
    if (env != nullptr)
    {
        AddIWADPath(env, "");
    }

    AddXdgDirs();
    AddSteamDirs();

    // Don't run this function again.
    iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
//
// DefinedIn: d_iwad.cpp
// UsedIn:    d_iwad.cpp / d_iwad.hpp
//            doom/d_main.cpp
std::string D_FindWADByName(const std::string_view name)
{
    // Absolute path?

    auto probe = M_FileCaseExists(name);
    if (!probe.empty())
    {
        return probe;
    }

    BuildIWADDirList();

    // Search through all IWAD paths for a file with the given name.

    for (auto &iwadDir : v_iwadDirs)
    {
        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD
        // file.

        probe = M_FileCaseExists(iwadDir.c_str());
        if (DirIsFile(iwadDir.c_str(), name.data()) && !probe.empty())
        {
            return probe;
        }

        // Construct a string for the full path
        char *path = M_StringJoin({iwadDir, DIR_SEPARATOR_S, name});

        probe = M_FileCaseExists(path);
        if (!probe.empty())
        {
            return probe;
        }
    }

    // File not found
    return std::string();
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or returns a copy of the filename
// if not found.
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    deh_main.cpp
//            w_main.cpp
std::string D_TryFindWADByName(const std::string_view filename)
{
    auto result = D_FindWADByName(filename);

    if (result.empty())
    {
        result = std::string(filename);
    }
    return result;
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    doom/d_main.cpp
std::string D_FindIWAD(int mask, GameMission_t *mission)
{
    auto result = std::string();
    // Check for the -iwad parameter

    //!
    // Specify an IWAD file to use.
    //
    // @arg <file>
    //

    int iwadparm = M_CheckParmWithArgs("-iwad", 1);

    if (iwadparm)
    {
        // Search through IWAD dirs for an IWAD with the given name.

        char *iwadfile = myargv[iwadparm + 1];

        result = D_FindWADByName(iwadfile);

        if (result.empty())
        {
            S_Error(fmt::format("IWAD file '{}' not found!", iwadfile));
        }

        *mission = IdentifyIWADByName(result.c_str(), mask);
    }
    else
    {
        // Search through the list and look for an IWAD

        BuildIWADDirList();

        for (auto &iwadDir : v_iwadDirs)
        {
            result = SearchDirectoryForIWAD(iwadDir.c_str(), mask, mission);
            if (!result.empty()) { break; }
        }
    }

    return result;
}

// Find all IWADs in the IWAD search path matching the given mask.
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    setup/mode.cpp
auto D_FindAllIWADs(int mask) -> const iwad_t **
{
    auto result = create_struct<iwad_t const * [a_iwads.size() + 1]>();
    //    result = malloc(sizeof(iwad_t *) * (arrlen(iwads) + 1));
    int result_len = 0;

    // Try to find all IWADs

    for (const auto &i : a_iwads)
    {
        if (((1 << i.mission) & mask) == 0)
        {
            continue;
        }

        auto filename = D_FindWADByName(i.name);

        if (!filename.empty())
        {
            result[result_len] = &i;
            ++result_len;
        }
    }

    // End of list

    result[result_len] = NULL;

    return result;
}

//
// Get the IWAD name used for savegames.
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    doom/d_main.cpp
auto D_SaveGameIWADName(GameMission_t gamemission) -> const char *
{
    // Determine the IWAD name to use for savegames.
    // This determines the directory the savegame files get put into.
    //
    // Note that we match on gamemission rather than on IWAD name.
    // This ensures that doom1.wad and doom.wad saves are stored
    // in the same place.

    for (const auto &i : a_iwads)
    {
        if (gamemission == i.mission)
        {
            return i.name;
        }
    }

    // Default fallback:

    return "unknown.wad";
}

// D_SuggestIWADName
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    setup/multiplayer.cpp
auto D_SuggestIWADName(GameMission_t mission, GameMode_t mode) -> const char *
{
    for (const auto &i : a_iwads)
    {
        if (i.mission == mission && i.mode == mode)
        {
            return i.name;
        }
    }

    return "unknown.wad";
}

// D_SuggestGameName
//
// DefinedIn: d_iwad.cpp  d_iwad.hpp
// UsedIn:    setup/multiplayer.cpp
//            w_main.cpp
auto D_SuggestGameName(GameMission_t mission, GameMode_t mode) -> const char *
{
    for (const auto &i : a_iwads)
    {
        if (i.mission == mission
            && (mode == GameMode_t::undetermined || i.mode == mode))
        {
            return i.description;
        }
    }

    return "Unknown game?";
}

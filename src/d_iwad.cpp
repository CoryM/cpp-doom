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

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "../utils/memory.hpp"
#include "d_iwad.hpp"
#include "deh_str.hpp"
#include "doomkeys.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

static const auto a_iwads = std::to_array<iwad_t>({
    { "doom2.wad", doom2, commercial, "Doom II" },
    { "plutonia.wad", pack_plut, commercial, "Final Doom: Plutonia Experiment" },
    { "tnt.wad", pack_tnt, commercial, "Final Doom: TNT: Evilution" },
    { "doom.wad", doom, retail, "Doom" },
    { "doom1.wad", doom, shareware, "Doom Shareware" },
    { "chex.wad", pack_chex, retail, "Chex Quest" },
    { "hacx.wad", pack_hacx, commercial, "Hacx" },
    { "freedoom2.wad", doom2, commercial, "Freedoom: Phase 2" },
    { "freedoom1.wad", doom, retail, "Freedoom: Phase 1" },
    { "freedm.wad", doom2, commercial, "FreeDM" },
    { "heretic.wad", heretic, retail, "Heretic" },
    { "heretic1.wad", heretic, shareware, "Heretic Shareware" },
    { "hexen.wad", hexen, commercial, "Hexen" },
    { "strife1.wad", strife, commercial, "Strife" }
});

// Helper function to get Enviroment Varibles into a string_view
std::string_view env_view(const char *envVar){
        const auto envCharStar = getenv(envVar);
        // Creating a string view with a nullprt is undefined behavour.
        if (envCharStar != nullptr) {
            return std::string_view(envCharStar);
        } 
        return std::string_view();
}

// Array of locations to search for IWAD files
//
static bool iwad_dirs_built = false;
auto v_iwadDirs = std::vector<std::string>();

// Returns true if the specified path is a path to a file
// of the specified name.
static bool DirIsFile(const char *path, const char *filename)
{
    return strchr(path, DIR_SEPARATOR) != NULL
           && !strcasecmp(M_BaseName(path), filename);
}

// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.
static char *CheckDirectoryHasIWAD(const char *dir, const char *iwadname)
{
    char *filename;
    char *probe;

    // As a special case, the "directory" may refer directly to an
    // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.

    probe = M_FileCaseExists(dir);
    if (DirIsFile(dir, iwadname) && probe != NULL)
    {
        return probe;
    }

    // Construct the full path to the IWAD if it is located in
    // this directory, and check if it exists.

    if (!strcmp(dir, "."))
    {
        filename = M_StringDuplicate(iwadname);
    }
    else
    {
        filename = M_StringJoin(dir, DIR_SEPARATOR_S, iwadname, NULL);
    }

    free(probe);
    probe = M_FileCaseExists(filename);
    free(filename);
    if (probe != NULL)
    {
        return probe;
    }

    return NULL;
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.

static char *SearchDirectoryForIWAD(const char *dir, int mask, GameMission_t *mission)
{
    char * filename;

    for (const auto &i : a_iwads)
    {
        if (((1 << i.mission) & mask) == 0)
        {
            continue;
        }

        filename = CheckDirectoryHasIWAD(dir, DEH_String(i.name));

        if (filename != nullptr)
        {
            *mission = i.mission;

            return filename;
        }
    }

    return NULL;
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.

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
static void AddIWADPath(const char *path, const char *suffix)
{
    char *left, *p, *dup_path;

    dup_path = M_StringDuplicate(path);

    // Split into individual dirs within the list.
    left = dup_path;

    for (;;)
    {
        p = strchr(left, PATH_SEPARATOR);
        if (p != NULL)
        {
            // Break at the separator and use the left hand side
            // as another iwad dir
            *p = '\0';

            v_iwadDirs.push_back(M_StringJoin(left, suffix, NULL));
            left = p + 1;
        }
        else
        {
            break;
        }
    }

    v_iwadDirs.push_back(M_StringJoin(left, suffix, NULL));

    free(dup_path);
}

// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static void AddXdgDirs(void)
{
    // Quote:
    // > $XDG_DATA_HOME defines the base directory relative to which
    // > user specific data files should be stored. If $XDG_DATA_HOME
    // > is either not set or empty, a default equal to
    // > $HOME/.local/share should be used.
        
    auto env = env_view("XDG_DATA_HOME");

    char *tmp_env = nullptr;

    if (env.data() == nullptr)
    {
        auto homedir = env_view("HOME");

        if (homedir.data() == nullptr)
        {
            homedir = std::basic_string_view<char>("~/");
        }

        tmp_env = M_StringJoin(homedir.data(), "/.local/share", NULL);
        env     = std::string_view(tmp_env);
    }

    // We support $XDG_DATA_HOME/games/doom (which will usually be
    // ~/.local/share/games/doom) as a user-writeable extension to
    // the usual /usr/share/games/doom location.
    v_iwadDirs.push_back(M_StringJoin(env.data(), "/games/doom", NULL));
    free(tmp_env);

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
static void AddSteamDirs(void)
{
    char *steampath;

    auto homedir = env_view("HOME");
    if (homedir.data() == nullptr)
    {
        homedir = std::string_view("/");
    }
    steampath = M_StringJoin(homedir.data(), "/.steam/root/steamapps/common", NULL);

    AddIWADPath(steampath, "/Doom 2/base");
    AddIWADPath(steampath, "/Master Levels of Doom/doom2");
    AddIWADPath(steampath, "/Ultimate Doom/base");
    AddIWADPath(steampath, "/Final Doom/base");
    AddIWADPath(steampath, "/DOOM 3 BFG Edition/base/wads");
    AddIWADPath(steampath, "/Heretic Shadow of the Serpent Riders/base");
    AddIWADPath(steampath, "/Hexen/base");
    AddIWADPath(steampath, "/Hexen Deathkings of the Dark Citadel/base");
    AddIWADPath(steampath, "/Strife");
    free(steampath);
}


//
// Build a list of IWAD files
//

static void BuildIWADDirList(void)
{
    char *env;

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
    env = getenv("DOOMWADDIR");
    if (env != nullptr)
    {
        v_iwadDirs.push_back(env);
    }

    // Add dirs from DOOMWADPATH:
    env = getenv("DOOMWADPATH");
    if (env != NULL)
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

char *D_FindWADByName(const char *name)
{
    char *path;
    char *probe;

    // Absolute path?

    probe = M_FileCaseExists(name);
    if (probe != NULL)
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
        if (DirIsFile(iwadDir.c_str(), name) && probe != NULL)
        {
            return probe;
        }
        free(probe);

        // Construct a string for the full path

        path = M_StringJoin(iwadDir.c_str(), DIR_SEPARATOR_S, name, NULL);

        probe = M_FileCaseExists(path);
        if (probe != NULL)
        {
            return probe;
        }

        free(path);
    }

    // File not found

    return NULL;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or returns a copy of the filename
// if not found.
//

char *D_TryFindWADByName(const char *filename)
{
    char *result;

    result = D_FindWADByName(filename);

    if (result != NULL)
    {
        return result;
    }
    else
    {
        return M_StringDuplicate(filename);
    }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

char *D_FindIWAD(int mask, GameMission_t *mission)
{
    char *result;
    char *iwadfile;
    int   iwadparm;

    // Check for the -iwad parameter

    //!
    // Specify an IWAD file to use.
    //
    // @arg <file>
    //

    iwadparm = M_CheckParmWithArgs("-iwad", 1);

    if (iwadparm)
    {
        // Search through IWAD dirs for an IWAD with the given name.

        iwadfile = myargv[iwadparm + 1];

        result = D_FindWADByName(iwadfile);

        if (result == NULL)
        {
            I_Error("IWAD file '%s' not found!", iwadfile);
        }

        *mission = IdentifyIWADByName(result, mask);
    }
    else
    {
        // Search through the list and look for an IWAD

        result = NULL;

        BuildIWADDirList();

        for (auto &iwadDir : v_iwadDirs)
        {
            result = SearchDirectoryForIWAD(iwadDir.c_str(), mask, mission);
            if (result) {break;}
        }
    }

    return result;
}

// Find all IWADs in the IWAD search path matching the given mask.

const iwad_t **D_FindAllIWADs(int mask)
{
    int   result_len;
    char *filename;

    auto result = create_struct<iwad_t const * [a_iwads.size() + 1]>();
    //    result = malloc(sizeof(iwad_t *) * (arrlen(iwads) + 1));
    result_len = 0;

    // Try to find all IWADs

    for (const auto &i : a_iwads)
    {
        if (((1 << i.mission) & mask) == 0)
        {
            continue;
        }

        filename = D_FindWADByName(i.name);

        if (filename != NULL)
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

const char *D_SaveGameIWADName(GameMission_t gamemission)
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

const char *D_SuggestIWADName(GameMission_t mission, GameMode_t mode)
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

const char *D_SuggestGameName(GameMission_t mission, GameMode_t mode)
{
    for (const auto &i : a_iwads)
    {
        if (i.mission == mission
            && (mode == indetermined || i.mode == mode))
        {
            return i.description;
        }
    }

    return "Unknown game?";
}

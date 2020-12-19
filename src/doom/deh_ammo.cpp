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
//
// Parses "Ammo" sections in dehacked files
//

#include <strings.h> // for strcasecmp
#include <cstdio>    // for NULL, sscanf
#include <cstdlib>   // for atoi
#include <string>    // for basic_string

#include "fmt/core.h"

#include "../deh_defs.hpp" // for deh_section_t
#include "../deh_io.hpp"   // for DEH_Warning
#include "../deh_main.hpp" // for DEH_ParseAssignment
#include "doomdef.hpp"     // for NUMAMMO
#include "p_local.hpp"     // for maxammo, clipammo
#include "sha1.hpp"        // for SHA1_UpdateInt32, sha1_context_t

struct deh_context_s;


static auto DEH_AmmoStart(deh_context_s *context, char *line) -> void *
{
    int ammo_number = 0;

    if (sscanf(line, "Ammo %i", &ammo_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return nullptr;
    }

    if (ammo_number < 0 || ammo_number >= NUMAMMO)
    {
        DEH_Warning(context, fmt::format("Invalid ammo number: {}", ammo_number));
        return nullptr;
    }

    return &maxammo[ammo_number];
}

static void DEH_AmmoParseLine(deh_context_s *context, char *line, void *tag)
{
    char *variable_name, *value;
    int   ivalue;
    int   ammo_number;

    if (tag == NULL)
        return;

    ammo_number = ((int *)tag) - maxammo;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    ivalue = atoi(value);

    // maxammo

    if (!strcasecmp(variable_name, "Per ammo"))
        clipammo[ammo_number] = ivalue;
    else if (!strcasecmp(variable_name, "Max ammo"))
        maxammo[ammo_number] = ivalue;
    else
    {
        DEH_Warning(context, fmt::format("Field named '{}' not found", variable_name));
    }
}

static void DEH_AmmoSHA1Hash(sha1_context_t *context)
{
    int i;

    for (i = 0; i < NUMAMMO; ++i)
    {
        SHA1_UpdateInt32(context, clipammo[i]);
        SHA1_UpdateInt32(context, maxammo[i]);
    }
}

deh_section_t deh_section_ammo = {
    "Ammo",
    NULL,
    DEH_AmmoStart,
    DEH_AmmoParseLine,
    NULL,
    DEH_AmmoSHA1Hash,
};

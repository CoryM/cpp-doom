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
// Parses "Sound" sections in dehacked files
//

#include "../doomtype.hpp"
#include "../deh_defs.hpp"
#include "../deh_main.hpp"
#include "../deh_mapping.hpp"
#include "sounds.hpp"

#include "fmt/core.h"

#include <cstdio>
#include <cstdlib>


DEH_BEGIN_MAPPING(sound_mapping, sfxinfo_t)
DEH_UNSUPPORTED_MAPPING("Offset")
DEH_UNSUPPORTED_MAPPING("Zero/One")
DEH_MAPPING("Value", priority)
DEH_MAPPING("Zero 1", link)
DEH_MAPPING("Zero 2", pitch)
DEH_MAPPING("Zero 3", volume)
DEH_UNSUPPORTED_MAPPING("Zero 4")
DEH_MAPPING("Neg. One 1", usefulness)
DEH_MAPPING("Neg. One 2", lumpnum)
DEH_END_MAPPING

static void *DEH_SoundStart(deh_context_s *context, char *line)
{
    int sound_number = 0;

    if (sscanf(line, "Sound %i", &sound_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return nullptr;
    }

    if (sound_number < 0 || sound_number >= NUMSFX)
    {
        DEH_Warning(context, fmt::format("Invalid sound number: {}", sound_number));
        return nullptr;
    }

    if (sound_number >= DEH_VANILLA_NUMSFX)
    {
        DEH_Warning(context, fmt::format("Attempt to modify SFX {}.  This will cause "
                                         "problems in Vanilla dehacked.",
                                 sound_number));
    }

    return &S_sfx[sound_number];
}

static void DEH_SoundParseLine(deh_context_s *context, char *line, void *tag)
{
    sfxinfo_t *sfx;
    char *     variable_name, *value;
    int        ivalue;

    if (tag == nullptr)
        return;

    sfx = (sfxinfo_t *)tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse
        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    // all values are integers

    ivalue = atoi(value);

    // Set the field value

    DEH_SetMapping(context, &sound_mapping, sfx, variable_name, ivalue);
}

deh_section_t deh_section_sound = {
    "Sound",
    NULL,
    DEH_SoundStart,
    DEH_SoundParseLine,
    NULL,
    NULL,
};

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
//	Main program, simply calls D_DoomMain high level loop.
//

#include "config.hpp"
#include "crispy.hpp"

#include <cstdio>
#include <iostream>

#include "SDL2/SDL.h"

#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp" // [crispy] M_snprintf()
#include "d_iwad.hpp"

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain(void);

int main(int argc, char **argv)
{
    // save arguments
    myargc = argc;
    myargv = argv;

    v_iwadDirs_init();

    //!
    // Print the program version and exit.
    //
    if (M_ParmExists("-version") || M_ParmExists("--version"))
    {
        puts(PACKAGE_STRING);
        exit(0);
    }

    {
        char        buf[16];
        SDL_version version;
        SDL_GetVersion(&version);
        M_snprintf(buf, sizeof(buf), "%d.%d.%d", version.major, version.minor, version.patch);
        crispy->sdlversion = M_StringDuplicate(buf);
        crispy->platform   = SDL_GetPlatform();
    }

    M_FindResponseFile();

#ifdef SDL_HINT_NO_SIGNAL_HANDLERS
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif

    // start doom

    D_DoomMain();
    
    // Cleanup
    std::cout << "Shutting down and cleaning up" << std::endl;
    v_iwadDirs_clear();

    return 0;
}

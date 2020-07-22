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
#include "d_iwad.hpp"
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp" // [crispy] M_snprintf()

#include "SDL2/SDL_mixer.h"
#include "fmt/core.h"

#include <iostream>

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//
void D_DoomMain();

auto main(int argc, char **argv) -> int
{
    try
    {
        // save arguments
        myArgs = c_Arguments(argc, argv);
        M_SetArgument(argc, argv);

        v_iwadDirs_init();

        //!
        // Print the program version and exit.
        //
        if (M_ParmExists("-version") || M_ParmExists("--version"))
        {
            puts(PACKAGE_STRING);
            //exit(0);
            throw std::logic_error(exceptionalExit);
        }

        {
            SDL_version version;
            SDL_GetVersion(&version);
            auto buf           = fmt::format("{}.{}.{}", version.major, version.minor, version.patch);
            crispy->sdlversion = M_StringDuplicate(buf);
            crispy->platform   = SDL_GetPlatform();
        }

        M_FindResponseFile();

#ifdef SDL_HINT_NO_SIGNAL_HANDLERS
        SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif

        // start doom

        D_DoomMain();
    } catch (std::exception &e)
    {
        std::cout << "Caught an Exception : " << e.what() << std::endl;
    }

    // Cleanup
    std::cout << "cleaning up" << std::endl;
    v_iwadDirs_clear();

    std::cout << "Shutting down" << std::endl;
    return 0;
}

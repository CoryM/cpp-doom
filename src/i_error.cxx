module;
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
//

#include <format>
#include <string_view>

#include "SDL.h"
#include "config.h"

#include "m_argv.hpp"
#include "i_system.hpp"

export module i_error;

//
// I_Error
//
export template<typename... Args>
[[noreturn]] void I_Error(std::string_view rt_fmt_str, Args&&... args)
{
    static bool already_quitting = false;

    if (already_quitting)
    {
        fprintf(stderr, "Warning: recursive call to I_Error detected.\n");
        exit(-1);
    }
    else
    {
        already_quitting = true;
    }

    // Message first.
    auto msgbuff =  std::vformat(rt_fmt_str, std::make_format_args(args...));

    // Shutdown. Here might be other errors.
    atexit_listentry_t *entry = get_exit_funcs();

    while (entry != nullptr)
    {
        if (entry->run_on_error)
        {
            entry->func();
        }

        entry = entry->next;
    }

    //!
    // @category obscure
    //
    // If specified, don't show a GUI window for error messages when the
    // game exits with an error.
    //
    bool exit_gui_popup = !M_ParmExists("-nogui");

    // Pop up a GUI dialog box to show the error message, if the
    // game was not run from the console (and the user will
    // therefore be unable to otherwise see the message).
    if (exit_gui_popup && !I_ConsoleStdout())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            PACKAGE_STRING, msgbuff.c_str(), nullptr);
    }

    SDL_Quit();

    exit(-1);
};

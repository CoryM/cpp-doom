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
#include <ranges>
#include <string_view>
#include <vector>

#include "SDL.h"
#include "config.h"

#include "m_argv.hpp"
#include "i_system.hpp"
#include "../utils/memory.hpp"

export module i_exit;

export using atexit_func_t = void (*)();

// Structure for a function to be called at exit.
struct atexit_listentry_t {
    atexit_func_t       func;
    bool             run_on_error;
};

// List of functions to be called at exit.
// Still has "side effects" but is not exported.
std::vector<atexit_listentry_t> exit_funcs;

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)
export void I_AtExit(atexit_func_t func, bool run_on_error)
{
    exit_funcs.emplace_back(func, run_on_error);
}

// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
export [[noreturn]] void I_Quit(void)
{
    // Run through all exit functions
    for (auto &entry : exit_funcs)
    {
        entry.func();
    }

    SDL_Quit();

    exit(0);
}


//
// I_Error_Impl
//
[[noreturn]] void I_Error_Impl(std::string msg)
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
            PACKAGE_STRING, msg.c_str(), nullptr);
    }

    // Shutdown. Here might be other errors.
    auto only_run_on_error = std::views::filter([](atexit_listentry_t i) { 
        return i.run_on_error && i.func != nullptr; 
    });

    for (auto &r : exit_funcs | only_run_on_error) {
        r.func();
    };

    SDL_Quit();

    exit(-1);
};


// Print error message and exit.
export template<typename... Args>
[[noreturn]] void I_Error(std::string_view rt_fmt_str, Args&&... args)
{
    auto msg =  std::vformat(rt_fmt_str, std::make_format_args(args...));
    I_Error_Impl(msg); // Does not return
};
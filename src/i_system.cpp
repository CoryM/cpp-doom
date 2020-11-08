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

#include "../utils/memory.hpp"
#include "config.hpp"
#include "deh_str.hpp"
#include "doomtype.hpp"
#include "i_joystick.hpp"
#include "i_sound.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "SDL2/SDL.h"
#include "fmt/core.h"

#include <iostream>
#include <string_view>
#include <unistd.h>


constexpr size_t DEFAULT_RAM = 16 * 2; /* MiB [crispy] */
constexpr size_t MIN_RAM     = 4 * 4;  /* MiB [crispy] */


typedef struct atexit_listentry_s atexit_listentry_t;

struct atexit_listentry_s {
    atexit_func_t       func;
    bool                run_on_error;
    atexit_listentry_t *next;
};

static atexit_listentry_t *exit_funcs = nullptr;

void I_AtExit(atexit_func_t func, bool run_on_error)
{
    auto *entry = create_struct<atexit_listentry_t>();

    entry->func         = func;
    entry->run_on_error = run_on_error;
    entry->next         = exit_funcs;
    exit_funcs          = entry;
}

// Tactile feedback function, probably used for the Logitech Cyberman

void I_Tactile(int on [[maybe_unused]], int off [[maybe_unused]], int total [[maybe_unused]])
{
}

// Zone memory auto-allocation function that allocates the zone size
// by trying progressively smaller zone sizes until one is found that
// works.

static auto AutoAllocMemory(size_t *size, int default_ram, int min_ram) -> byte *
{
    // Allocate the zone memory.  This loop tries progressively smaller
    // zone sizes until a size is found that can be allocated.
    // If we used the -mb command line parameter, only the parameter
    // provided is accepted.

    byte *zonemem = nullptr;

    while (zonemem == nullptr)
    {
        // We need a reasonable minimum amount of RAM to start.

        if (default_ram < min_ram)
        {
            S_Error(fmt::format("Unable to allocate {} MiB of RAM for zone", default_ram));
        }

        // Try to allocate the zone memory.
        constexpr size_t KiloBytes = 1024;
        constexpr size_t MegaBytes = 1024 * KiloBytes;
        *size                      = default_ram * MegaBytes;

        zonemem = static_cast<byte *>(malloc(*size));

        // Failed to allocate?  Reduce zone size until we reach a size
        // that is acceptable.

        if (zonemem == nullptr)
        {
            default_ram -= 1;
        }
    }

    return zonemem;
}

auto I_ZoneBase(size_t *size) -> byte *
{
    int        min_ram, default_ram;
    int        p;
    static int i = 1;

    //!
    // @category obscure
    // @arg <mb>
    //
    // Specify the heap size, in MiB (default 16).
    //

    p = M_CheckParm("-mb", 1);

    if (p > 0)
    {
        default_ram = M_GetArgumentAsInt(p + 1);
        min_ram     = default_ram;
    }
    else
    {
        default_ram = DEFAULT_RAM;
        min_ram     = MIN_RAM;
    }

    // [crispy] do not allocate new zones ad infinitum
    if (i > 8)
    {
        min_ram = default_ram + 1;
    }

    byte *zonemem = AutoAllocMemory(size, default_ram * i, min_ram * i);

    // [crispy] if called again, allocate another zone twice as big
    i *= 2;

    constexpr size_t MegaByteBitSift = 20;
    fmt::print("zone memory: {}, {} MiB allocated for zone\n",
        *zonemem,
        (*size >> MegaByteBitSift)); // [crispy] human-understandable zone heap size

    return zonemem;
}

void I_PrintBanner(const char *msg)
{
    int i;
    int spaces = 35 - (strlen(msg) / 2);

    for (i = 0; i < spaces; ++i)
        putchar(' ');

    puts(msg);
}

void I_PrintDivider(void)
{
    int i;

    for (i = 0; i < 75; ++i)
    {
        putchar('=');
    }

    putchar('\n');
}

void I_PrintStartupBanner(const char *gamedescription)
{
    I_PrintDivider();
    I_PrintBanner(gamedescription);
    I_PrintDivider();

    printf(
        " " PACKAGE_NAME " is free software, covered by the GNU General Public\n"
        " License.  There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
        " FOR A PARTICULAR PURPOSE. You are welcome to change and distribute\n"
        " copies under certain conditions. See the source for more information.\n");

    I_PrintDivider();
}

//
// I_ConsoleStdout
//
// Returns true if stdout is a real console, false if it is a file
//

bool I_ConsoleStdout(void)
{
    return isatty(fileno(stdout));
}

//
// I_Init
//
/*
void I_Init (void)
{
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
}
void I_BindVariables(void)
{
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();
}
*/

//
// I_Quit
//

void I_Quit(void)
{
    atexit_listentry_t *entry;

    // Run through all exit functions

    entry = exit_funcs;

    while (entry != NULL)
    {
        entry->func();
        entry = entry->next;
    }

    SDL_Quit();

    //exit(0);
    throw std::logic_error(exceptionalExit);
}


//
// I_Error
//
static bool already_quitting = false;

void I_Error(const char *error, ...)
{
    char                msgbuf[512];
    va_list             argptr;
    atexit_listentry_t *entry;
    bool                exit_gui_popup;

    if (already_quitting)
    {
        fmt::print(stderr, "Warning: recursive call to I_Error detected.\n");
        throw std::logic_error(exceptionalExit);
    }
    else
    {
        already_quitting = true;
    }

    // Message first.
    va_start(argptr, error);
    //fprintf(stderr, "\nError: ");
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
    va_end(argptr);
    fflush(stderr);

    // Write a copy of the message into buffer.
    va_start(argptr, error);
    memset(msgbuf, 0, sizeof(msgbuf));
    M_vsnprintf(msgbuf, sizeof(msgbuf), error, argptr);
    va_end(argptr);

    // Shutdown. Here might be other errors.

    entry = exit_funcs;

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
    exit_gui_popup = !M_ParmExists("-nogui");

    // Pop up a GUI dialog box to show the error message, if the
    // game was not run from the console (and the user will
    // therefore be unable to otherwise see the message).
    if (exit_gui_popup && !I_ConsoleStdout())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            PACKAGE_STRING, msgbuf, NULL);
    }

    // abort();

    SDL_Quit();
    throw std::logic_error(exceptionalExit);
}

void S_Error(const std::string_view error)
{
    if (!already_quitting)
    {
        already_quitting = true;
    }
    else
    {
        std::cerr << "Warning: recursive call to I_Error detected.\n";
        throw std::logic_error(exceptionalExit);
    }

    // Message first.
    std::cerr << "\nError: " << error;

    // Shutdown. Here might be other errors.

    atexit_listentry_t *entry = exit_funcs;

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
    // Pop up a GUI dialog box to show the error message, if the
    // game was not run from the console (and the user will
    // therefore be unable to otherwise see the message).
    if ((!M_ParmExists("-nogui")) && (!I_ConsoleStdout()))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            PACKAGE_STRING, error.data(), nullptr);
    }

    // abort();

    SDL_Quit();

    //exit(-1);
    throw std::logic_error(exceptionalExit);
}

//
// I_Realloc
//
void *I_Realloc(void *ptr, size_t size)
{
    void *new_ptr;

    new_ptr = realloc(ptr, size);

    if (size != 0 && new_ptr == NULL)
    {
        S_Error(fmt::format("I_Realloc: failed on reallocation of {} bytes", size));
    }

    return new_ptr;
}

//
// Read Access Violation emulation.
//
// From PrBoom+, by entryway.
//

// C:\>debug
// -d 0:0
//
// DOS 6.22:
// 0000:0000  (57 92 19 00) F4 06 70 00-(16 00)
// DOS 7.1:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// Win98:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// DOSBox under XP:
// 0000:0000  (00 00 00 F1) ?? ?? ?? 00-(07 00)

#define DOS_MEM_DUMP_SIZE 10

static const unsigned char mem_dump_dos622[DOS_MEM_DUMP_SIZE] = {
    0x57, 0x92, 0x19, 0x00, 0xF4, 0x06, 0x70, 0x00, 0x16, 0x00
};
static const unsigned char mem_dump_win98[DOS_MEM_DUMP_SIZE] = {
    0x9E, 0x0F, 0xC9, 0x00, 0x65, 0x04, 0x70, 0x00, 0x16, 0x00
};
static const unsigned char mem_dump_dosbox[DOS_MEM_DUMP_SIZE] = {
    0x00, 0x00, 0x00, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00
};
static unsigned char mem_dump_custom[DOS_MEM_DUMP_SIZE];

static const unsigned char *dos_mem_dump = mem_dump_dos622;

bool I_GetMemoryValue(unsigned int offset, void *value, int size)
{
    static bool firsttime = true;

    if (firsttime)
    {
        int p, i, val;

        firsttime = false;
        i         = 0;

        //!
        // @category compat
        // @arg <version>
        //
        // Specify DOS version to emulate for NULL pointer dereference
        // emulation.  Supported versions are: dos622, dos71, dosbox.
        // The default is to emulate DOS 7.1 (Windows 98).
        //

        p = M_CheckParm("-setmem", 1);

        if (p > 0)
        {
            if (!strcasecmp(M_GetArgument(p + 1).data(), "dos622"))
            {
                dos_mem_dump = mem_dump_dos622;
            }
            if (!strcasecmp(M_GetArgument(p + 1).data(), "dos71"))
            {
                dos_mem_dump = mem_dump_win98;
            }
            else if (!strcasecmp(M_GetArgument(p + 1).data(), "dosbox"))
            {
                dos_mem_dump = mem_dump_dosbox;
            }
            else
            {
                for (i = 0; i < DOS_MEM_DUMP_SIZE; ++i)
                {
                    ++p;

                    if (p >= M_GetArgumentCount() || M_GetArgument(p)[0] == '-')
                    {
                        break;
                    }

                    //M_StrToInt(myargv[p], &val);
                    val                  = M_GetArgumentAsInt(p);
                    mem_dump_custom[i++] = (unsigned char)val;
                }

                dos_mem_dump = mem_dump_custom;
            }
        }
    }

    switch (size)
    {
    case 1:
        *((unsigned char *)value) = dos_mem_dump[offset];
        return true;
    case 2:
        *((unsigned short *)value) = dos_mem_dump[offset]
                                     | (dos_mem_dump[offset + 1] << 8);
        return true;
    case 4:
        *((unsigned int *)value) = dos_mem_dump[offset]
                                   | (dos_mem_dump[offset + 1] << 8)
                                   | (dos_mem_dump[offset + 2] << 16)
                                   | (dos_mem_dump[offset + 3] << 24);
        return true;
    }

    return false;
}

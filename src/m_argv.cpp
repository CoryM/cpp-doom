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
#include "m_argv.hpp" // haleyjd 20110212: warning fix

#include "common.hpp"
#include "d_iwad.hpp"
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"

#include "fmt/core.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <string_view>

int         myargc;
char **     myargv;
c_Arguments myArgs;

constexpr size_t MAXARGVS = 100;


c_Arguments::c_Arguments(int newArgC, char **newArgV)
{
    importArguments(std::span(newArgV, newArgC));
}


void c_Arguments::importArguments(const std::span<char *> &arrayOfArgs)
{
    for (auto &a : arrayOfArgs)
    {
        m_Args.emplace_back(std::string(a));
    }
}


auto c_Arguments::at(const int pos) -> std::string_view
{
    auto result = std::string_view();
    if (pos >= 0)
    {
        result = m_Args.at(pos);
    }
    return result;
}

auto c_Arguments::asInt32(const int pos) -> int
{
    int result = 0;
    if (pos >= 0)
    {
        result = atoi(m_Args.at(pos).data());
    }
    return result;
}

auto c_Arguments::size() -> size_t
{
    return m_Args.size();
}

auto c_Arguments::find(const std::string_view findMe, const int num_args) -> int
{
    int        index = c_Arguments::NotFound;
    const auto it    = std::find(m_Args.begin(), m_Args.end(), findMe);
    // Consider it found IF we did not reach the end AND we have enough args remaining
    if (it != m_Args.end())
    {
        if (static_cast<int>(m_Args.end() - it) > num_args)
        {
            index = static_cast<int>(it - m_Args.begin());
        }
    }
    return index;
}


void M_SetArgument(const int newArgC, char **newArgV)
{
    myargc = newArgC;
    myargv = newArgV;
}


auto M_GetArgument(const int arg) -> std::string_view
{
    return myArgs.at(arg);
}

auto M_GetArgumentAsInt(int arg) -> int32_t
{
    return myArgs.asInt32(arg);
}

auto M_GetArgumentCount() -> int
{
    return myArgs.size();
}


//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (0 to argc-1)
// or c_Arguments::NotFound (-1) if not present
auto M_CheckParm(const std::string_view check, int num_args /* = 0 */) -> int
{
    return myArgs.find(check, num_args);
}


//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//
auto M_ParmExists(const char *check) -> bool
{
    return M_CheckParm(check) != c_Arguments::NotFound;
}

static void LoadResponseFile(int argv_index, const char *filename)
{
    // Read the response file into memory
    auto handle = std::ifstream(filename, std::ios::binary);

    if (!handle.is_open())
    {
        fmt::print("\nNo such response file!");
        //exit(1);
        throw std::logic_error(std::string("exceptional exit ") + MACROS::LOCATION_STR);
    }

    fmt::print("Found response file {}!\n", filename);

    auto GetFileSize = [](auto &fileHandle) {
        auto currentPos = fileHandle.tellg(); // Save current stream postion

        fileHandle.seekg(0, std::ios_base::end); // Goto the end of the stream
        auto endPos = fileHandle.tellg();        // Save the postion at the end

        fileHandle.seekg(0, std::ios_base::beg); // Goto the start of the stream
        auto startPos = fileHandle.tellg();      // Save the postion of the stream

        fileHandle.seekg(currentPos, std::ios_base::beg); // Restore file postion
        return (endPos - startPos);
    };

    auto size = GetFileSize(handle);
    fmt::print("file size : {}\n", size);

    // Read in the entire file
    // Allocate one byte extra - this is in case there is an argument
    // at the end of the response file, in which case a '\0' will be
    // needed.

    auto *file = static_cast<char *>(malloc(size + 1));

    for (int i = 0; i < size; i++)
    {
        //auto k = fread(file + i, 1, size - i, handle);
        handle.get(&file[i], size - i);
        auto k = handle.gcount();

        if (k < 0)
        {
            S_Error(fmt::format("Failed to read full contents of '{}'", filename));
        }

        i += k;
    }

    handle.close();

    // Create new arguments list array

    auto **newargv = static_cast<char **>(malloc(sizeof(char *) * MAXARGVS));
    int    newargc = 0;
    memset(newargv, 0, sizeof(char *) * MAXARGVS);

    // Copy all the arguments in the list up to the response file

    for (int i = 0; i < argv_index; ++i, ++newargc)
    {
        newargv[i] = myargv[i];
    }

    auto *infile = file;

    for (int k = 0; k < size;)
    {
        // Skip past space characters to the next argument

        while (k < size && (std::isspace(infile[k]) != 0))
        {
            ++k;
        }

        if (k >= size)
        {
            break;
        }

        // If the next argument is enclosed in quote marks, treat
        // the contents as a single argument.  This allows long filenames
        // to be specified.

        if (infile[k] == '\"')
        {
            // Skip the first character(")
            ++k;

            newargv[newargc++] = &infile[k];

            // Read all characters between quotes

            while (k < size && infile[k] != '\"' && infile[k] != '\n')
            {
                ++k;
            }

            if (k >= size || infile[k] == '\n')
            {
                S_Error(fmt::format("Quotes unclosed in response file '{}'", filename));
            }

            // Cut off the string at the closing quote

            infile[k] = '\0';
            ++k;
        }
        else
        {
            // Read in the next argument until a space is reached

            newargv[newargc++] = &infile[k];

            while (k < size && !isspace(infile[k]))
            {
                ++k;
            }

            // Cut off the end of the argument at the first space

            infile[k] = '\0';

            ++k;
        }
    }

    // Add arguments following the response file argument

    for (int i = argv_index + 1; i < myargc; ++i)
    {
        newargv[newargc] = myargv[i];
        ++newargc;
    }

    myargv = newargv;
    myargc = newargc;

    // Disabled - Vanilla Doom does not do this.
    // Display arguments

    fmt::print("{} command-line args:\n", myargc);

    for (int k = 0; k < myargc; k++)
    {
        fmt::print("{}:'{}'\n", k, myargv[k]);
    }
}

//
// Find a Response File
//

void M_FindResponseFile()
{
    int i;

    for (i = 1; i < myargc; i++)
    {
        if (myargv[i][0] == '@')
        {
            LoadResponseFile(i, myargv[i] + 1);
        }
    }

    for (;;)
    {
        //!
        // @arg <filename>
        //
        // Load extra command line arguments from the given response file.
        // Arguments read from the file will be inserted into the command
        // line replacing this argument. A response file can also be loaded
        // using the abbreviated syntax '@filename.rsp'.
        //
        i = M_CheckParm("-response", 1);
        if (i <= 0)
        {
            break;
        }
        // Replace the -response argument so that the next time through
        // the loop we'll ignore it. Since some parameters stop reading when
        // an argument beginning with a '-' is encountered, we keep something
        // that starts with a '-'.
        myargv[i] = "-_";
        LoadResponseFile(i + 1, M_GetArgument(i + 1).data());
    }
}

// Return the name of the executable used to start the program:

const char *M_GetExecutableName(void)
{
    return M_BaseName(myargv[0]);
}

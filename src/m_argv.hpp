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
//  Nil.
//


#ifndef __M_ARGV__
#define __M_ARGV__

#include "doomtype.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

//
// MISC
//
void M_SetArgument(int newArgC, char **newArgV);

// Same as M_CheckParm, but checks that num_args arguments are available
// following the specified argument.
auto M_CheckParm(const std::string_view check, int num_args = 0) -> int;
auto M_GetArgument(int arg) -> std::string_view;
auto M_GetArgumentAsInt(int arg) -> int32_t;
auto M_GetArgumentCount() -> int;

void M_FindResponseFile();
void M_AddLooseFiles();

// Parameter has been specified?

auto M_ParmExists(const char *check) -> bool;

// Get name of executable used to run this program:

auto M_GetExecutableName() -> const char *;

// class index_t {
// private:
//     uint32_t m_index = Unknown;
//
// public:
//     enum : uint32_t
//     {
//         Ok         = 0x00000000,
//         Unknown    = 0x10000000,
//         NotFound   = 0x20000000,
//         OutOfRange = 0x30000000,
//         ErrorMask  = 0xF0000000,
//         IndexMask  = 0x0fffffff
//     };
//     index_t() = default;
//
//     explicit index_t(uint32_t val)
//         : m_index(val) {};
//
//     [[nodiscard]] auto index() const -> uint32_t
//     {
//         return m_index & IndexMask;
//     };
//
//     [[nodiscard]] auto isGood() const -> bool
//     {
//         return (m_index & ErrorMask) == Ok;
//     };
//
//     [[nodiscard]] auto isBad() const -> bool
//     {
//         return (m_index & ErrorMask) != Ok;
//     };
//
//     [[nodiscard]] auto isUnkown() const -> bool
//     {
//         return (m_index & ErrorMask) == Unknown;
//     };
//
//     [[nodiscard]] auto isNotFound() const -> bool
//     {
//         return (m_index & ErrorMask) == NotFound;
//     };
//
//     [[nodiscard]] auto isOutOfRange() const -> bool
//     {
//         return (m_index & ErrorMask) == OutOfRange;
//     };
// };

class c_Arguments {
private:
    std::vector<std::string> m_Args;

public:
    enum : int
    {
        NotFound = -1
    };
    c_Arguments() = default;
    c_Arguments(int newArgC, char **newArgV);
    //~c_Arguments() = default;
    void importArguments(const std::span<char *> &arrayOfArgs);
    auto at(const int pos) -> std::string_view;
    auto asInt32(const int pos) -> int;
    auto size() -> size_t;
    auto find(const std::string_view findMe, const int num_args) -> int;
};

extern c_Arguments myArgs;

#endif

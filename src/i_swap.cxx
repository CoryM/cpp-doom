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
//	Endianess handling, swapping 16bit and 32bit.
//

// Use C++20's std::endian
#include <bit>      // std::endian std::byteswap
#include <cstdint>  // int16_t, int32_t

export module i_swap;

export namespace endian {

bool consteval is_big_endian()
{
    return std::endian::native == std::endian::big;
};

bool consteval is_little_endian()
{
    return std::endian::native == std::endian::little;
};

template <typename T>
T big(T value)
{
    if constexpr (is_little_endian())
    {
        return std::byteswap(value);
    }
    else
    {
        return value;
    }
};

template <typename T>
T little(T value)
{
    if constexpr (is_big_endian())
    {
        return std::byteswap(value);
    }
    else
    {
        return value;
    }
};

int16_t SHORT(int16_t value)
{
    return little(value);
};

int32_t LONG(int32_t value)
{
    return little(value);
};


} // namespace endian
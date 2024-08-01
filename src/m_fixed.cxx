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
//	Fixed point arithemtics, implementation.
//
// m_fixed.c and n_fixed,c converted to C++ module m_fixed.cxx

#include <cstdlib> // abs
#include <limits>  // std::numeric_limits
#include <cstdint> // int64_t

export module m_fixed;


//
// Fixed point, 32bit as 16.16.
//
export typedef int    fixed_t;
export constexpr auto FRACBITS = 16;
export constexpr auto FRACUNIT = (1 << FRACBITS);
export constexpr bool SafeFixed = true;
export constexpr auto FIXED2DOUBLE(fixed_t x) -> double
{
    return x / static_cast<double>(FRACUNIT);
};

export constexpr fixed_t FixedMul(const fixed_t a, const fixed_t b)
{
    if constexpr (SafeFixed)
    {
        const int64_t val = (static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> FRACBITS;
        if (val > std::numeric_limits<int32_t>::max())
        {
            return std::numeric_limits<int32_t>::max();
        }
        else if (val < std::numeric_limits<int32_t>::min())
        {
            return std::numeric_limits<int32_t>::min();
        }
        return val;
    } else {
        return (static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> FRACBITS;
    };
}


//
// FixedDiv, C version.
//

export constexpr fixed_t FixedDiv(const fixed_t a, const fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
        return (a ^ b) < 0 ? std::numeric_limits<int32_t>::min() : std::numeric_limits<int32_t>::max();
    }
    else
    {
        int64_t result = (static_cast<int64_t>(a) << FRACBITS) / b;

        return static_cast<fixed_t>(result);
    }
}

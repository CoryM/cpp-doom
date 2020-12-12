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
//	Fixed point implementation.
//

#include "m_fixed.hpp"

#include <climits> // INT_MIN and INT_MAX
#include <cstdlib> // int64_t


// Fixme. __USE_C_FIXED__ or something.
auto FixedMul(const fixed_t a, const fixed_t b) -> fixed_t
{
    return ((int64_t)a * (int64_t)b) >> FRACBITS;
}

//
// FixedDiv, C version.
//
auto FixedDiv(const fixed_t a, const fixed_t b) -> fixed_t
{
    constexpr unsigned int MAGIC_BITSHIFT_NUMBER = 14;
    if ((abs(a) >> MAGIC_BITSHIFT_NUMBER) >= abs(b))
    {
        return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    }

    return static_cast<fixed_t>((static_cast<int64_t>(a) << FRACBITS) / b);
}

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

module;

#include <cstdint>
#include <cmath>

export module m_fixed;

//
// Fixed point, 32bit as 16.16.
//
export using fixed_t = int32_t;

export constexpr int32_t FRACBITS        = 16;
export constexpr int32_t FRACUNIT        = (1 << FRACBITS);

export double FIXED2DOUBLE(fixed_t x) {
    return static_cast<double>(x) / FRACUNIT;
};

// Fixme. __USE_C_FIXED__ or something.

export fixed_t FixedMul(fixed_t a,
        fixed_t      b)
{
    return ((int64_t)a * (int64_t)b) >> FRACBITS;
}


export fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
        return (a ^ b) < 0 ? INT32_MIN : INT32_MAX;
    }
    else
    {
        int64_t result;

        result = ((int64_t)a << FRACBITS) / b;

        return (fixed_t)result;
    }
}
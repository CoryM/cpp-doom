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
#if 0
    constexpr auto FixedScale = static_cast<double>(1 << FRACBITS);
    const auto     dA         = static_cast<double>(a);
    const auto     dB         = static_cast<double>(b);
    const auto     Product    = (dA * dB) / FixedScale;
    return static_cast<fixed_t>(Product);
#else
    return ((int64_t)a * (int64_t)b) >> FRACBITS;
#endif
}

//
// FixedDiv, C version.
//
auto FixedDiv(const fixed_t a, const fixed_t b) -> fixed_t
{
#if 0
    constexpr auto FixedScale = static_cast<double>(1 << FRACBITS);
    const auto     dA         = static_cast<double>(a);
    const auto     dB         = static_cast<double>(b);
    const auto     Quotient   = (dA / dB) * FixedScale;

    return static_cast<fixed_t>(Quotient);
#else
    constexpr unsigned int MAGIC_BITSHIFT_NUMBER = 14;
    if ((abs(a) >> MAGIC_BITSHIFT_NUMBER) >= abs(b))
    {
        return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    }

    return static_cast<fixed_t>((static_cast<int64_t>(a) << FRACBITS) / b);
#endif
}

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
//	Common Standard and 3rd party Libraries
//

#pragma once
#ifndef __COMMON_HPP__
#define __COMMON_HPP__

// C Standard Libraries (C++ Version)
#include <cctype>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>

// C++ Standard Libraries
#include <array>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// 3rd Party Libraries
#include "SDL2/SDL.h"

// OS Libraries
#include <unistd.h>

namespace MACROS {
using std::string;
#define S1(x)        #x
#define S2(x)        S1(x)
#define LOCATION_STR string("In File " __FILE__ " on line " S2(__LINE__))
}

#endif
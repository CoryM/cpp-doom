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

//#include <string>
#include "fmt/core.h"

namespace MACROS {
#define exceptionalExit fmt::format("exceptional exit in File {} on line {}", __FILE__, __LINE__)
}

#endif
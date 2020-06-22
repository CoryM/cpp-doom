//
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
//
// Dehacked string replacements
//

#ifndef DEH_STR_H
#define DEH_STR_H

#include "common.hpp"

#include "doomtype.hpp"

// Used to do dehacked text substitutions throughout the program

const char *DEH_String(const std::string_view s);
void        DEH_printf(const char *fmt, ...) PRINTF_ATTR(1, 2);
void        DEH_fprintf(FILE *fstream, const char *fmt, ...) PRINTF_ATTR(2, 3);
void        DEH_snprintf(char *buffer, size_t len, const char *fmt, ...) PRINTF_ATTR(3, 4);
void        DEH_AddStringReplacement(const std::string_view from_text, const std::string_view to_text);
bool        DEH_HasStringReplacement(const std::string_view s);

#endif /* #ifndef DEH_STR_H */

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2018 Fabian Greffrath
// Copyright(C) 2018 Julia Nechaevskaya
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
//	[crispy] Crispness menu tiled background
//

#include "m_background.hpp"

#include <array>

/*

The following array contains a seamlessly tiling 64x64 icon depicting
Crisps to use as a background texture for the Crispness menu. It has
been converted from a raw lump in Doom's flat format with the following
code. The actual artwork has been created by Julia Nechaevskaya, thank
you so incredibly much for this! - Fabian

#include <cstdio>

int main (int argc, char **argv)
{
	FILE *file;
	int c, i = 0;

	if (argc < 2 || !(file = fopen(argv[1], "r")))
	{
		return -1;
	}

	while ((c = getc(file)) != EOF)
	{
		printf("0x%02x, ", c);
		if (!(++i % 64))
		{
			printf("\n");
		}
	}

	fclose(file);
	return 0;
}
*/

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
//	System specific interface stuff.
//


#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.hpp"
#include "crispy.hpp"

// Screen width and height.

constexpr int ORIGWIDTH  = 320; // [crispy]
constexpr int ORIGHEIGHT = 200; // [crispy]

constexpr auto MAXWIDTH  = static_cast<unsigned int>(ORIGWIDTH) << 2U;  // [crispy]
constexpr auto MAXHEIGHT = static_cast<unsigned int>(ORIGHEIGHT) << 1U; // [crispy]

extern unsigned int SCREENWIDTH;
extern unsigned int SCREENHEIGHT;
extern unsigned int HIRESWIDTH;                      // [crispy] non-widescreen SCREENWIDTH
extern int          DELTAWIDTH;                      // [crispy] horizontal widescreen offset
auto                I_GetScreenDimensions() -> void; // [crispy] re-calculate DELTAWIDTH

// Screen height used when aspect_ratio_correct=true.

constexpr int ORIGHEIGHT_4_3 = 240;                                             // [crispy]
constexpr int MAXHEIGHT_4_3  = static_cast<unsigned int>(ORIGHEIGHT_4_3) << 1U; // [crispy]

extern unsigned int SCREENHEIGHT_4_3;

typedef bool (*grabmouse_callback_t)();

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
auto I_InitGraphics() -> void;

auto I_GraphicsCheckCommandLine() -> void;

auto I_ShutdownGraphics() -> void;

// Takes full 8 bit values.
auto           I_SetPalette(int palette) -> void;
extern pixel_t I_MapRGB(const uint8_t r, const uint8_t g, const uint8_t b);

auto I_UpdateNoBlit() -> void;
auto I_FinishUpdate() -> void;

auto I_ReadScreen(pixel_t *scr) -> void;

auto I_BeginRead() -> void;

auto I_SetWindowTitle(const char *title) -> void;

auto I_CheckIsScreensaver() -> void;
auto I_SetGrabMouseCallback(grabmouse_callback_t func) -> void;

auto I_DisplayFPSDots(bool dots_on) -> void;
auto I_BindVideoVariables() -> void;

auto I_InitWindowTitle() -> void;
auto I_InitWindowIcon() -> void;

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

auto I_StartFrame() -> void;

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

auto I_StartTic() -> void;

// Enable the loading disk image displayed when reading from disk.

auto I_EnableLoadingDisk(int xoffs, int yoffs) -> void;

extern char *video_driver;
extern bool  screenvisible;

extern int      vanilla_keyboard_mapping;
extern bool     screensaver_mode;
extern int      usegamma;
extern pixel_t *I_VideoBuffer;

extern int screen_width;
extern int screen_height;
extern int fullscreen;
extern int aspect_ratio_correct;
extern int integer_scaling;
extern int vga_porch_flash;
extern int force_software_renderer;

extern char *window_position;
auto         I_GetWindowPosition(int *x, int *y, int w, int h) -> void;

// Joystic/gamepad hysteresis
extern unsigned int joywait;

#endif

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
//	DOOM graphics stuff for SDL.
//


#include "SDL2/SDL.h"

#include "icon.cpp"

#include "crispy.hpp"

#include "../utils/lump.hpp"
#include "config.hpp"
#include "d_loop.hpp"
#include "deh_str.hpp"
#include "doomtype.hpp"
#include "i_input.hpp"
#include "i_joystick.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "tables.hpp"
#include "v_diskicon.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include <algorithm>

int SCREENWIDTH, SCREENHEIGHT, SCREENHEIGHT_4_3;
int HIRESWIDTH; // [crispy] non-widescreen SCREENWIDTH
int DELTAWIDTH; // [crispy] horizontal widescreen offset

// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

static SDL_Window *  screen;
static SDL_Renderer *renderer;

// Window title

static const char *window_title = "";

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

static SDL_Surface *argbbuffer       = nullptr;
static SDL_Texture *texture          = nullptr;
static SDL_Texture *texture_upscaled = nullptr;

static uint32_t pixel_format;

// palette

static SDL_Texture * curpane = nullptr;
static SDL_Texture * redpane = nullptr;
static SDL_Texture * yelpane = nullptr;
static SDL_Texture * grnpane = nullptr;
static int           pane_alpha;
static unsigned int  rmask, gmask, bmask, amask; // [crispy] moved up here
static const uint8_t blend_alpha = 0xa8;
extern pixel_t *     colormaps; // [crispy] evil hack to get FPS dots working as in Vanilla
static bool          palette_to_set;

// display has been set up?

static bool initialized = false;

// disable mouse?

static bool nomouse  = false;
int         usemouse = 1;

// Save screenshots in PNG format.

int png_screenshots = 1; // [crispy]

// SDL video driver name

char *video_driver = "";

// Window position:

char *window_position = "center";

// SDL display number on which to run.

int video_display = 0;

// Screen width and height, from configuration file.

int window_width  = 800;
int window_height = 600;

// Fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int fullscreen_width = 0, fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int max_scaling_buffer_pixels = 16000000;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// Aspect ratio correction mode

int        aspect_ratio_correct = true;
static int actualheight;

// Force integer scales for resolution-independent rendering

int integer_scaling = false;

// VGA Porch palette change emulation

int vga_porch_flash = false;

// Force software rendering, for systems which lack effective hardware
// acceleration

int force_software_renderer = false;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code). nograbmouse_override allows
// this to be temporarily disabled via the command line.

static int  grabmouse            = true;
static bool nograbmouse_override = false;

// The screen buffer; this is modified to draw things to the screen

pixel_t *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

bool screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

bool screenvisible = true;

// If true, we display dots at the bottom of the screen to
// indicate FPS.

static bool display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static bool noblit;

// Callback function to invoke to determine whether to grab the
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = nullptr;

// Does the window currently have focus?

static bool window_focused = true;

// Window resize state.

static bool         need_resize = false;
static unsigned int last_resize_time;
constexpr int       RESIZE_DELAY = 500;

// Gamma correction level to use

int usegamma = 0;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;

static auto MouseShouldBeGrabbed() -> bool
{
    // never grab the mouse when in screensaver mode

    if (screensaver_mode)
    {
        return false;
    }

    // if the window doesn't have focus, never grab it

    if (!window_focused)
    {
        return false;
    }

    // always grab the mouse when full screen (dont want to
    // see the mouse pointer)

    if (fullscreen)
    {
        return true;
    }

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
    {
        return false;
    }

    // if we specify not to grab the mouse, never grab

    if (nograbmouse_override || !grabmouse)
    {
        return false;
    }

    // Invoke the grabmouse callback function to determine whether
    // the mouse should be grabbed

    if (grabmouse_callback != NULL)
    {
        return grabmouse_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(bool dots_on)
{
    display_fps_dots = dots_on;
}

static void SetShowCursor(bool show)
{
    if (!screensaver_mode)
    {
        // When the cursor is hidden, grab the input.
        // Relative mode implicitly hides the cursor.
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(!show));
        SDL_GetRelativeMouseState(NULL, NULL);
    }
}

auto I_ShutdownGraphics() -> void
{
    if (initialized)
    {
        SetShowCursor(true);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        initialized = false;
    }
}


//
// I_StartFrame
//
auto I_StartFrame() -> void
{
    // er?
}

// Adjust window_width / window_height variables to be an an aspect
// ratio consistent with the aspect_ratio_correct variable.
static auto AdjustWindowSize() -> void
{
    if (aspect_ratio_correct || integer_scaling)
    {
        if (window_width * actualheight <= window_height * SCREENWIDTH)
        {
            // We round up window_height if the ratio is not exact; this leaves
            // the result stable.
            window_height = (window_width * actualheight + SCREENWIDTH - 1) / SCREENWIDTH;
        }
        else
        {
            window_width = window_height * SCREENWIDTH / actualheight;
        }
    }
}

static auto HandleWindowEvent(SDL_WindowEvent *event) -> void
{
    int i;

    switch (event->event)
    {
#if 0 // SDL2-TODO
        case SDL_ACTIVEEVENT:
            // need to update our focus state
            UpdateFocus();
            break;
#endif
    case SDL_WINDOWEVENT_EXPOSED:
        palette_to_set = true;
        break;

    case SDL_WINDOWEVENT_RESIZED:
        need_resize      = true;
        last_resize_time = SDL_GetTicks();
        break;

        // Don't render the screen when the window is minimized:

    case SDL_WINDOWEVENT_MINIMIZED:
        screenvisible = false;
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
    case SDL_WINDOWEVENT_RESTORED:
        screenvisible = true;
        break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.

    case SDL_WINDOWEVENT_FOCUS_GAINED:
        window_focused = true;
        break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
        window_focused = false;
        break;

        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the video_display config variable.

    case SDL_WINDOWEVENT_MOVED:
        i = SDL_GetWindowDisplayIndex(screen);
        if (i >= 0)
        {
            video_display = i;
        }
        break;

    default:
        break;
    }
}

static auto ToggleFullScreenKeyShortcut(SDL_Keysym *sym) -> bool
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return (sym->scancode == SDL_SCANCODE_RETURN || sym->scancode == SDL_SCANCODE_KP_ENTER) && (sym->mod & flags) != 0;
}

static auto I_ToggleFullScreen() -> void
{
    unsigned int flags = 0;

    // TODO: Consider implementing fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (fullscreen_width != 0 || fullscreen_height != 0)
    {
        return;
    }

    fullscreen = !fullscreen;

    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);

    if (!fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, window_width, window_height);
    }
}

auto I_GetEvent() -> void
{
    extern void I_HandleKeyboardEvent(SDL_Event * sdlevent);
    extern void I_HandleMouseEvent(SDL_Event * sdlevent);
    SDL_Event   sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
        case SDL_KEYDOWN:
            if (ToggleFullScreenKeyShortcut(&sdlevent.key.keysym))
            {
                I_ToggleFullScreen();
                break;
            }
            // deliberate fall-though

        case SDL_KEYUP:
            I_HandleKeyboardEvent(&sdlevent);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
            if (usemouse && !nomouse && window_focused)
            {
                I_HandleMouseEvent(&sdlevent);
            }
            break;

        case SDL_QUIT:
            if (screensaver_mode)
            {
                I_Quit();
            }
            else
            {
                event_t event;
                event.type = evtype_t::ev_quit;
                D_PostEvent(&event);
            }
            break;

        case SDL_WINDOWEVENT:
            if (sdlevent.window.windowID == SDL_GetWindowID(screen))
            {
                HandleWindowEvent(&sdlevent.window);
            }
            break;

        default:
            break;
        }
    }
}

//
// I_StartTic
//
auto I_StartTic() -> void
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();

    if (usemouse && !nomouse && window_focused)
    {
        I_ReadMouse();
    }

    if (joywait < I_GetTime())
    {
        I_UpdateJoystick();
    }
}


//
// I_UpdateNoBlit
//
auto I_UpdateNoBlit() -> void
{
    // what is this?
}

static auto UpdateGrab() -> void
{
    static bool currently_grabbed = false;
    bool        grab;

    grab = MouseShouldBeGrabbed();

    if (screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
    }
    else if (!grab && currently_grabbed)
    {
        int screen_w, screen_h;

        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_GetWindowSize(screen, &screen_w, &screen_h);
        SDL_WarpMouseInWindow(screen, screen_w - 16, screen_h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;
}

static auto LimitTextureSize(int *w_upscale, int *h_upscale) -> void
{
    SDL_RendererInfo rinfo;

    int orig_w = *w_upscale;
    int orig_h = *h_upscale;

    // Query renderer and limit to maximum texture dimensions of hardware:
    if (SDL_GetRendererInfo(renderer, &rinfo) != 0)
    {
        S_Error(fmt::format("CreateUpscaledTexture: SDL_GetRendererInfo() call failed: {}",
            SDL_GetError()));
    }

    while (*w_upscale * SCREENWIDTH > rinfo.max_texture_width)
    {
        --*w_upscale;
    }
    while (*h_upscale * SCREENHEIGHT > rinfo.max_texture_height)
    {
        --*h_upscale;
    }

    if ((*w_upscale < 1 && rinfo.max_texture_width > 0) || (*h_upscale < 1 && rinfo.max_texture_height > 0))
    {
        S_Error(fmt::format("CreateUpscaledTexture: Can't create a texture big enough for "
                            "the whole screen! Maximum texture size {}x{}",
            rinfo.max_texture_width, rinfo.max_texture_height));
    }

    // We limit the amount of texture memory used for the intermediate buffer,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very
    // huge textures, so the user can use this to reduce the maximum texture
    // size if desired.

    if (max_scaling_buffer_pixels < SCREENWIDTH * SCREENHEIGHT)
    {
        S_Error(fmt::format("CreateUpscaledTexture: max_scaling_buffer_pixels too small "
                            "to create a texture buffer: {} < {}",
            max_scaling_buffer_pixels, SCREENWIDTH * SCREENHEIGHT));
    }

    while (*w_upscale * *h_upscale * SCREENWIDTH * SCREENHEIGHT
           > max_scaling_buffer_pixels)
    {
        if (*w_upscale > *h_upscale)
        {
            --*w_upscale;
        }
        else
        {
            --*h_upscale;
        }
    }

    if (*w_upscale != orig_w || *h_upscale != orig_h)
    {
        printf("CreateUpscaledTexture: Limited texture size to %dx%d "
               "(max %d pixels, max texture size %dx%d)\n",
            *w_upscale * SCREENWIDTH, *h_upscale * SCREENHEIGHT,
            max_scaling_buffer_pixels,
            rinfo.max_texture_width, rinfo.max_texture_height);
    }
}

static auto CreateUpscaledTexture(bool force) -> void
{
    // Get the size of the renderer output. The units this gives us will be
    // real world pixels, which are not necessarily equivalent to the screen's
    // window size (because of highdpi).
    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        S_Error(fmt::format("Failed to get renderer output size: {}", SDL_GetError()));
    }

    // When the screen or window dimensions do not match the aspect ratio
    // of the texture, the rendered area is scaled down to fit. Calculate
    // the actual dimensions of the rendered area.

    if (w * actualheight < h * SCREENWIDTH)
    {
        // Tall window.

        h = w * actualheight / SCREENWIDTH;
    }
    else
    {
        // Wide window.

        w = h * SCREENWIDTH / actualheight;
    }

    // Pick texture size the next integer multiple of the screen dimensions.
    // If one screen dimension matches an integer multiple of the original
    // resolution, there is no need to overscale in this direction.

    int w_upscale = (w + SCREENWIDTH - 1) / SCREENWIDTH;
    int h_upscale = (h + SCREENHEIGHT - 1) / SCREENHEIGHT;

    // Minimum texture dimensions of 320x200.

    if (w_upscale < 1)
    {
        w_upscale = 1;
    }
    if (h_upscale < 1)
    {
        h_upscale = 1;
    }

    LimitTextureSize(&w_upscale, &h_upscale);

    // Create a new texture only if the upscale factors have actually changed.
    static int h_upscale_old, w_upscale_old;
    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force)
    {
        return;
    }

    h_upscale_old = h_upscale;
    w_upscale_old = w_upscale;

    // Set the scaling quality for rendering the upscaled texture to "linear",
    // which looks much softer and smoother than "nearest" but does a better
    // job at downscaling from the upscaled texture to screen.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    auto *new_texture = SDL_CreateTexture(renderer,
        pixel_format,
        SDL_TEXTUREACCESS_TARGET,
        w_upscale * SCREENWIDTH,
        h_upscale * SCREENHEIGHT);

    SDL_Texture *texture_upscaled;
    SDL_Texture *old_texture      = texture_upscaled;
    SDL_Texture *texture_upscaled = new_texture;

    if (old_texture != nullptr)
    {
        SDL_DestroyTexture(old_texture);
    }
}

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
fixed_t fractionaltic;

//
// I_FinishUpdate
//
auto I_FinishUpdate() -> void
{
    int i;

    if (!initialized)
    {
        return;
    }

    if (noblit)
    {
        return;
    }

    if (need_resize)
    {
        if (SDL_GetTicks() > last_resize_time + RESIZE_DELAY)
        {
            // When the window is resized (we're not in fullscreen mode),
            // save the new window size.
            int flags = SDL_GetWindowFlags(screen);
            if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0)
            {
                SDL_GetWindowSize(screen, &window_width, &window_height);

                // Adjust the window by resizing again so that the window
                // is the right aspect ratio.
                AdjustWindowSize();
                SDL_SetWindowSize(screen, window_width, window_height);
            }
            CreateUpscaledTexture(false);
            need_resize    = false;
            palette_to_set = true;
        }
        else
        {
            return;
        }
    }

    UpdateGrab();

    // draws little dots on the bottom of the screen

    if (display_fps_dots)
    {
        static int lasttic;
        auto       i       = I_GetTime();
        int        tics    = i - lasttic;
        int        lasttic = i;

        if (tics > 20) { tics = 20 };

        for (i = 0; i < tics * 4; i += 4)
        {
            I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = colormaps[0xff];
        }
        for (; i < 20 * 4; i += 4)
        {
            I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = colormaps[0x0];
        }
    }

    // [crispy] [AM] Real FPS counter
    {
        static int lastmili = 0;
        static int fpscount = 0;

        fpscount++;

        i        = SDL_GetTicks();
        int mili = i - lastmili;

        // Update FPS counter every second
        if (mili >= 1000)
        {
            crispy->fps = (fpscount * 1000) / mili;
            fpscount    = 0;
            lastmili    = i;
        }
    }

    // Draw disk icon before blit, if necessary.
    V_DrawDiskIcon();

    // Update the intermediate texture with the contents of the RGBA buffer.
    SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);

    // Make sure the pillarboxes are kept clear each frame.
    SDL_RenderClear(renderer);

    if (crispy->smoothscaling)
    {
        // Render this intermediate texture into the upscaled texture
        // using "nearest" integer scaling.

        SDL_SetRenderTarget(renderer, texture_upscaled);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Finally, render this upscaled texture to screen using linear scaling.

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    }
    else
    {
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }

    if (curpane)
    {
        SDL_SetTextureAlphaMod(curpane, pane_alpha);
        SDL_RenderCopy(renderer, curpane, NULL, NULL);
    }

    // Draw!

    SDL_RenderPresent(renderer);

    // [AM] Figure out how far into the current tic we're in as a fixed_t.
    if (crispy->uncapped)
    {
        fractionaltic = I_GetTimeMS() * TICRATE % 1000 * FRACUNIT / 1000;
    }

    // Restore background and undo the disk indicator, if it was drawn.
    V_RestoreDiskBackground();
}


//
// I_ReadScreen
//
void I_ReadScreen(pixel_t *scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT * sizeof(*scr));
}


//
// I_SetPalette
//
// [crispy] intermediate gamma levels
byte **gamma2table = nullptr;
auto   I_SetGammaTable() -> void
{
    gamma2table = static_cast<byte **>(malloc(9 * sizeof(*gamma2table)));

    // [crispy] 5 original gamma levels
    for (int i = 0; i < 5; i++)
    {
        gamma2table[2 * i] = (byte *)gammatable[i];
    }

    // [crispy] 4 intermediate gamma levels
    for (int i = 0; i < 4; i++)
    {
        gamma2table[2 * i + 1] = static_cast<byte *>(malloc(256 * sizeof(**gamma2table)));

        for (int j = 0; j < 256; j++)
        {
            gamma2table[2 * i + 1][j] = (gamma2table[2 * i][j] + gamma2table[2 * i + 2][j]) / 2;
        }
    }
}


void I_SetPalette(int palette)
{
    switch (palette)
    {
    case 0:
        curpane = NULL;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        curpane    = redpane;
        pane_alpha = 0xff * palette / 9;
        break;
    case 9:
    case 10:
    case 11:
    case 12:
        curpane    = yelpane;
        pane_alpha = 0xff * (palette - 8) / 8;
        break;
    case 13:
        curpane    = grnpane;
        pane_alpha = 0xff * 125 / 1000;
        break;
    default:
        S_Error(fmt::format("Unknown palette: {}!\n", palette));
        break;
    }
}

//
// Set the window title
//

void I_SetWindowTitle(const char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on
// the title set with I_SetWindowTitle.
//

auto I_InitWindowTitle() -> void
{
    char *buf = M_StringJoin({ window_title, " - ", PACKAGE_STRING });
    SDL_SetWindowTitle(screen, buf);
    free(buf);
}

// Set the application icon

auto I_InitWindowIcon() -> void
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *)icon_data, icon_w, icon_h,
        32, icon_w * 4,
        0xff << 24, 0xff << 16,
        0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(screen, surface);
    SDL_FreeSurface(surface);
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static auto SetScaleFactor(int factor) -> void
{
    // Pick 320x200 or 320x240, depending on aspect ratio correct

    window_width  = factor * SCREENWIDTH;
    window_height = factor * actualheight;
    fullscreen    = false;
}

auto I_GraphicsCheckCommandLine() -> void
{
    //!
    // @category video
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_ParmExists("-noblit");

    //!
    // @category video
    //
    // Don't grab the mouse when running in windowed mode.
    //

    nograbmouse_override = M_ParmExists("-nograbmouse");

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    //!
    // @category video
    //
    // Run in a window.
    //

    if (M_ParmExists("-window") || M_ParmExists("-nofullscreen"))
    {
        fullscreen = false;
    }

    //!
    // @category video
    //
    // Run in fullscreen mode.
    //

    if (M_ParmExists("-fullscreen"))
    {
        fullscreen = true;
    }

    //!
    // @category video
    //
    // Disable the mouse.
    //

    nomouse = M_ParmExists("-nomouse");

    //!
    // @category video
    // @arg <x>
    //
    // Specify the screen width, in pixels. Implies -window.
    //

    int i = M_CheckParm("-width", 1);

    if (i > 0)
    {
        window_width = M_GetArgumentAsInt(i + 1);
        fullscreen   = false;
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels. Implies -window.
    //

    i = M_CheckParm("-height", 1);

    if (i > 0)
    {
        window_height = M_GetArgumentAsInt(i + 1);
        fullscreen    = false;
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the dimensions of the window. Implies -window.
    //

    i = M_CheckParm("-geometry", 1);

    if (i > 0)
    {
        int w, h;

        int s = sscanf(M_GetArgument(i + 1).data(), "%ix%i", &w, &h);
        if (s == 2)
        {
            window_width  = w;
            window_height = h;
            fullscreen    = false;
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen. Implies -window.
    //

    if (M_ParmExists("-1"))
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size. Implies -window.
    //

    if (M_ParmExists("-2"))
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size. Implies -window.
    //

    if (M_ParmExists("-3"))
    {
        SetScaleFactor(3);
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

auto I_CheckIsScreensaver() -> void
{
    char *env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

static auto SetSDLVideoDriver() -> void
{
    // Allow a default value for the SDL video driver to be specified
    // in the configuration file.

    if (strcmp(video_driver, "") != 0)
    {
        char *env_string = M_StringJoin({ "SDL_VIDEODRIVER=", video_driver });
        putenv(env_string);
        free(env_string);
    }
}

// Check the display bounds of the display referred to by 'video_display' and
// set x and y to a location that places the window in the center of that
// display.
static auto CenterWindow(int *x, int *y, int w, int h) -> void
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
                        "for display #%d!\n",
            video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

auto I_GetWindowPosition(int *x, int *y, int w, int h) -> void
{
    // Check that video_display corresponds to a display that really exists,
    // and if it doesn't, reset it.
    if (video_display < 0 || video_display >= SDL_GetNumVideoDisplays())
    {
        fprintf(stderr,
            "I_GetWindowPosition: We were configured to run on display #%d, "
            "but it no longer exists (max %d). Moving to display 0.\n",
            video_display, SDL_GetNumVideoDisplays() - 1);
        video_display = 0;
    }

    // in fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run fullscreen on.

    if (fullscreen)
    {
        CenterWindow(x, y, w, h);
        return;
    }

    // in windowed mode, the desired window position can be specified
    // in the configuration file.

    if (window_position == NULL || !strcmp(window_position, ""))
    {
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
    else if (!strcmp(window_position, "center"))
    {
        // Note: SDL has a SDL_WINDOWPOS_CENTER, but this is useless for our
        // purposes, since we also want to control which display we appear on.
        // So we have to do this ourselves.
        CenterWindow(x, y, w, h);
    }
    else if (sscanf(window_position, "%i,%i", x, y) != 2)
    {
        // invalid format: revert to default
        fprintf(stderr, "I_GetWindowPosition: invalid window_position setting\n");
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
}

static auto SetVideoMode() -> void
{
    int             x, y;
    int             unused_bpp;
    int             renderer_flags = 0;
    SDL_DisplayMode mode;

    int w = window_width;
    int h = window_height;

    // In windowed mode, the window can be resized while the game is
    // running.
    int window_flags = SDL_WINDOW_RESIZABLE;

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    if (fullscreen)
    {
        if (fullscreen_width == 0 && fullscreen_height == 0)
        {
            // This window_flags means "Never change the screen resolution!
            // Instead, draw to the entire screen by scaling the texture
            // appropriately".
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            w = fullscreen_width;
            h = fullscreen_height;
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    // Running without window decorations is potentially useful if you're
    // playing in three window mode and want to line up three game windows
    // next to each other on a single desktop.
    // Deliberately not documented because I'm not sure how useful this is yet.
    if (M_ParmExists("-borderless"))
    {
        window_flags |= SDL_WINDOW_BORDERLESS;
    }

    I_GetWindowPosition(&x, &y, w, h);

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If
    // "window_flags" contains the fullscreen flag (see above), then
    // w and h are ignored.

    if (screen == nullptr)
    {
        screen = SDL_CreateWindow(NULL, x, y, w, h, window_flags);

        if (screen == nullptr)
        {
            S_Error(fmt::format("Error creating window for video startup: {}",
                SDL_GetError()));
        }

        pixel_format = SDL_GetWindowPixelFormat(screen);

        SDL_SetWindowMinimumSize(screen, SCREENWIDTH, actualheight);

        I_InitWindowTitle();
        I_InitWindowIcon();
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.
    renderer_flags = SDL_RENDERER_TARGETTEXTURE;

    if (SDL_GetCurrentDisplayMode(video_display, &mode) != 0)
    {
        S_Error(fmt::format("Could not get display mode for video display #{}: {}",
            video_display, SDL_GetError()));
    }

    // Turn on vsync if we aren't in a -timedemo
    if (!singletics && mode.refresh_rate > 0)
    {
        if (crispy->vsync) // [crispy] uncapped vsync
        {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
    }

    if (force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;
        crispy->vsync = false;
    }

    if (renderer != nullptr)
    {
        SDL_DestroyRenderer(renderer);
        // all associated textures get destroyed
        texture          = nullptr;
        texture_upscaled = nullptr;
    }

    renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

    // If we could not find a matching render driver,
    // try again without hardware acceleration.

    if (renderer == nullptr && !force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

        // If this helped, save the setting for later.
        if (renderer != nullptr)
        {
            force_software_renderer = 1;
        }
    }

    if (renderer == nullptr)
    {
        S_Error(fmt::format("Error creating renderer for screen window: {}",
            SDL_GetError()));
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    if (aspect_ratio_correct || integer_scaling)
    {
        SDL_RenderSetLogicalSize(renderer,
            SCREENWIDTH,
            actualheight);
    }

    // Force integer scales for resolution-independent rendering.

#if SDL_VERSION_ATLEAST(2, 0, 5)
    SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(integer_scaling));
#endif

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Format of argbbuffer must match the screen pixel format because we
    // import the surface data into the texture.

    if (argbbuffer != NULL)
    {
        SDL_FreeSurface(argbbuffer);
        argbbuffer = NULL;
    }

    if (argbbuffer == NULL)
    {
        SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
            &rmask, &gmask, &bmask, &amask);
        argbbuffer = SDL_CreateRGBSurface(0,
            SCREENWIDTH, SCREENHEIGHT, 32,
            rmask, gmask, bmask, amask);
        SDL_FillRect(argbbuffer, NULL, I_MapRGB(0xff, 0x0, 0x0));
        redpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(redpane, SDL_BLENDMODE_BLEND);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(0xd7, 0xba, 0x45));
        yelpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(yelpane, SDL_BLENDMODE_BLEND);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(0x0, 0xff, 0x0));
        grnpane = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(grnpane, SDL_BLENDMODE_BLEND);
        SDL_FillRect(argbbuffer, NULL, 0);
    }

    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
        pixel_format,
        SDL_TEXTUREACCESS_STREAMING,
        SCREENWIDTH, SCREENHEIGHT);

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture(true);
}

// [crispy] re-calculate SCREENWIDTH, SCREENHEIGHT, HIRESWIDTH and DELTAWIDTH
void I_GetScreenDimensions(void)
{
    SDL_DisplayMode mode;
    int             w = 16, h = 10;
    int             ah;

    SCREENWIDTH  = ORIGWIDTH << crispy->hires;
    SCREENHEIGHT = ORIGHEIGHT << crispy->hires;

    HIRESWIDTH = SCREENWIDTH;

    ah = (aspect_ratio_correct == 1) ? (6 * SCREENHEIGHT / 5) : SCREENHEIGHT;

    if (SDL_GetCurrentDisplayMode(video_display, &mode) == 0)
    {
        // [crispy] sanity check: really widescreen display?
        if (mode.w * ah >= mode.h * SCREENWIDTH)
        {
            w = mode.w;
            h = mode.h;
        }
    }

    // [crispy] widescreen rendering makes no sense without aspect ratio correction
    if (crispy->widescreen && aspect_ratio_correct)
    {
        SCREENWIDTH = w * ah / h;
        // [crispy] make sure SCREENWIDTH is an integer multiple of 4 ...
        SCREENWIDTH = (SCREENWIDTH + 3) & (int)~3;
        // [crispy] ... but never exceeds MAXWIDTH (array size!)
        SCREENWIDTH = std::min(SCREENWIDTH, MAXWIDTH);
    }

    DELTAWIDTH = ((SCREENWIDTH - HIRESWIDTH) >> crispy->hires) / 2;
}

void I_InitGraphics(void)
{
    SDL_Event dummy;
    char *    env;

    // Pass through the XSCREENSAVER_WINDOW environment variable to
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        char winenv[30];
        int  winid;

        sscanf(env, "0x%x", &winid);
        M_snprintf(winenv, sizeof(winenv), "SDL_WINDOWID=%i", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        S_Error(fmt::format("Failed to initialize video: {}", SDL_GetError()));
    }

    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    if (screensaver_mode)
    {
        fullscreen = true;
    }

    // [crispy] run-time variable high-resolution rendering
    I_GetScreenDimensions();

    // [crispy] (re-)initialize resolution-agnostic patch drawing
    V_Init();

    if (aspect_ratio_correct == 1)
    {
        actualheight = 6 * SCREENHEIGHT / 5;
    }
    else
    {
        actualheight = SCREENHEIGHT;
    }

    // Create the game window; this may switch graphic modes depending
    // on configuration.
    AdjustWindowSize();
    SetVideoMode();

    // SDL2-TODO UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (fullscreen && !screensaver_mode)
    {
        SDL_Delay(startup_delay);
    }

    // The actual 320x200 canvas that we draw to. This is the pixel buffer of
    // the 8-bit paletted screen buffer that gets blit on an intermediate
    // 32-bit RGBA screen buffer that gets loaded into a texture that gets
    // finally rendered into our window or full screen in I_FinishUpdate().

    I_VideoBuffer = static_cast<pixel_t *>(argbbuffer->pixels);
    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));

    // clear out any events waiting at the start and center the mouse

    while (SDL_PollEvent(&dummy))
        ;

    initialized = true;

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);
}

// [crispy] re-initialize only the parts of the rendering stack that are really necessary

void I_ReInitGraphics(int reinit)
{
    // [crispy] re-set rendering resolution and re-create framebuffers
    if (reinit & REINIT_FRAMEBUFFERS)
    {
        unsigned int rmask, gmask, bmask, amask;
        int          unused_bpp;

        I_GetScreenDimensions();

        // [crispy] re-initialize resolution-agnostic patch drawing
        V_Init();

        SDL_FreeSurface(argbbuffer);
        SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
            &rmask, &gmask, &bmask, &amask);
        argbbuffer    = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, rmask, gmask, bmask, amask);
        I_VideoBuffer = static_cast<pixel_t *>(argbbuffer->pixels);
        V_RestoreBuffer();

        // [crispy] it will get re-created below with the new resolution
        SDL_DestroyTexture(texture);
    }

    // [crispy] re-create renderer
    if (reinit & REINIT_RENDERER)
    {
        SDL_RendererInfo info = { 0 };
        int              flags;

        SDL_GetRendererInfo(renderer, &info);
        flags = info.flags;

        if (crispy->vsync && !(flags & SDL_RENDERER_SOFTWARE))
        {
            flags |= SDL_RENDERER_PRESENTVSYNC;
        }
        else
        {
            flags &= ~SDL_RENDERER_PRESENTVSYNC;
        }

        SDL_DestroyRenderer(renderer);
        renderer = SDL_CreateRenderer(screen, -1, flags);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // [crispy] the texture gets destroyed in SDL_DestroyRenderer(), force its re-creation
        texture_upscaled = NULL;
    }

    // [crispy] re-create textures
    if (reinit & REINIT_TEXTURES)
    {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

        texture = SDL_CreateTexture(renderer,
            pixel_format,
            SDL_TEXTUREACCESS_STREAMING,
            SCREENWIDTH, SCREENHEIGHT);

        // [crispy] force its re-creation
        CreateUpscaledTexture(true);
    }

    // [crispy] re-set logical rendering resolution
    if (reinit & REINIT_ASPECTRATIO)
    {
        if (aspect_ratio_correct == 1)
        {
            actualheight = 6 * SCREENHEIGHT / 5;
        }
        else
        {
            actualheight = SCREENHEIGHT;
        }

        if (aspect_ratio_correct || integer_scaling)
        {
            SDL_RenderSetLogicalSize(renderer,
                SCREENWIDTH,
                actualheight);
        }
        else
        {
            SDL_RenderSetLogicalSize(renderer, 0, 0);
        }

#if SDL_VERSION_ATLEAST(2, 0, 5)
        SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(integer_scaling));
#endif
    }

    // [crispy] adjust the window size and re-set the palette
    need_resize = true;
}

// [crispy] take screenshot of the rendered image

void I_RenderReadPixels(byte **data, int *w, int *h, int *p)
{
    SDL_Rect         rect;
    SDL_PixelFormat *format;
    int              temp;
    uint32_t         png_format;
    byte *           pixels;

    // [crispy] adjust cropping rectangle if necessary
    rect.x = rect.y = 0;
    SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    if (aspect_ratio_correct || integer_scaling)
    {
        if (integer_scaling)
        {
            int temp1, temp2, scale;
            temp1 = rect.w;
            temp2 = rect.h;
            scale = std::min(rect.w / SCREENWIDTH, rect.h / actualheight);

            rect.w = SCREENWIDTH * scale;
            rect.h = actualheight * scale;

            rect.x = (temp1 - rect.w) / 2;
            rect.y = (temp2 - rect.h) / 2;
        }
        else if (rect.w * actualheight > rect.h * SCREENWIDTH)
        {
            temp   = rect.w;
            rect.w = rect.h * SCREENWIDTH / actualheight;
            rect.x = (temp - rect.w) / 2;
        }
        else if (rect.h * SCREENWIDTH > rect.w * actualheight)
        {
            temp   = rect.h;
            rect.h = rect.w * actualheight / SCREENWIDTH;
            rect.y = (temp - rect.h) / 2;
        }
    }

    // [crispy] native PNG pixel format
#if SDL_VERSION_ATLEAST(2, 0, 5)
    png_format = SDL_PIXELFORMAT_RGB24;
#else
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    png_format = SDL_PIXELFORMAT_ABGR8888;
#else
    png_format = SDL_PIXELFORMAT_RGBA8888;
#endif
#endif
    format = SDL_AllocFormat(png_format);
    temp   = rect.w * format->BytesPerPixel; // [crispy] pitch

    // [crispy] As far as I understand the issue, SDL_RenderPresent()
    // may return early, i.e. before it has actually finished rendering the
    // current texture to screen -- from where we want to capture it.
    // However, it does never return before it has finished rendering the
    // *previous* texture.
    // Thus, we add a second call to SDL_RenderPresent() here to make sure
    // that it has at least finished rendering the previous texture, which
    // already contains the scene that we actually want to capture.
    if (crispy->post_rendering_hook)
    {
        SDL_RenderCopy(renderer, crispy->smoothscaling ? texture_upscaled : texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // [crispy] allocate memory for screenshot image
    pixels = static_cast<byte *>(malloc(rect.h * temp));
    SDL_RenderReadPixels(renderer, &rect, format->format, pixels, temp);

    *data = pixels;
    *w    = rect.w;
    *h    = rect.h;
    *p    = temp;

    SDL_FreeFormat(format);
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("use_mouse", &usemouse);
    M_BindIntVariable("fullscreen", &fullscreen);
    M_BindIntVariable("video_display", &video_display);
    M_BindIntVariable("aspect_ratio_correct", &aspect_ratio_correct);
    M_BindIntVariable("integer_scaling", &integer_scaling);
    M_BindIntVariable("vga_porch_flash", &vga_porch_flash);
    M_BindIntVariable("startup_delay", &startup_delay);
    M_BindIntVariable("fullscreen_width", &fullscreen_width);
    M_BindIntVariable("fullscreen_height", &fullscreen_height);
    M_BindIntVariable("force_software_renderer", &force_software_renderer);
    M_BindIntVariable("max_scaling_buffer_pixels", &max_scaling_buffer_pixels);
    M_BindIntVariable("window_width", &window_width);
    M_BindIntVariable("window_height", &window_height);
    M_BindIntVariable("grabmouse", &grabmouse);
    M_BindStringVariable("video_driver", &video_driver);
    M_BindStringVariable("window_position", &window_position);
    M_BindIntVariable("usegamma", &usegamma);
    M_BindIntVariable("png_screenshots", &png_screenshots);
}

const pixel_t I_BlendAdd(const pixel_t bg, const pixel_t fg)
{
    uint32_t r, g, b;

    if ((r = (fg & rmask) + (bg & rmask)) > rmask) r = rmask;
    if ((g = (fg & gmask) + (bg & gmask)) > gmask) g = gmask;
    if ((b = (fg & bmask) + (bg & bmask)) > bmask) b = bmask;

    return amask | r | g | b;
}

// [crispy] http://stereopsis.com/doubleblend.html
const pixel_t I_BlendDark(const pixel_t bg, const int d)
{
    const uint32_t ag = (bg & 0xff00ff00) >> 8;
    const uint32_t rb = bg & 0x00ff00ff;

    uint32_t sag = d * ag;
    uint32_t srb = d * rb;

    sag = sag & 0xff00ff00;
    srb = (srb >> 8) & 0x00ff00ff;

    return amask | sag | srb;
}

const pixel_t I_BlendOver(const pixel_t bg, const pixel_t fg)
{
    const uint32_t r = ((blend_alpha * (fg & rmask) + (0xff - blend_alpha) * (bg & rmask)) >> 8) & rmask;
    const uint32_t g = ((blend_alpha * (fg & gmask) + (0xff - blend_alpha) * (bg & gmask)) >> 8) & gmask;
    const uint32_t b = ((blend_alpha * (fg & bmask) + (0xff - blend_alpha) * (bg & bmask)) >> 8) & bmask;

    return amask | r | g | b;
}

const pixel_t (*blendfunc)(const pixel_t fg, const pixel_t bg) = I_BlendOver;

pixel_t I_MapRGB(const uint8_t r, const uint8_t g, const uint8_t b)
{
    /*
	return amask |
	        (((r * rmask) >> 8) & rmask) |
	        (((g * gmask) >> 8) & gmask) |
	        (((b * bmask) >> 8) & bmask);
*/
    return SDL_MapRGB(argbbuffer->format, r, g, b);
}

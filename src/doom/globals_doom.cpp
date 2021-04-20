#include "globals_doom.hpp"

namespace globals {

namespace doom {
    // am_map.cpp
    constinit bool       automapactive = false;
    constinit cheatseq_t cheat_amap    = CHEAT("iddt", 0);

    // Pulled from m_menu.cpp
    constinit bool inhelpscreens = false;
    constinit bool menuactive    = false;

    // r_draw.cpp
    constinit int viewwidth = 0;

} // namespace doom

} // namespace globals

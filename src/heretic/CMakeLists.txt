add_library(heretic STATIC)

target_sources(heretic 
    PRIVATE
        am_data.hpp
        am_map.cpp            
        ct_chat.cpp           
        deh_ammo.cpp          
        deh_frame.cpp
        deh_htext.cpp
        deh_htic.cpp          
        deh_sound.cpp
        deh_thing.cpp
        deh_weapon.cpp
        d_main.cpp
        d_net.cpp
        f_finale.cpp
        g_game.cpp
        info.cpp
        in_lude.cpp
        m_random.cpp          
        mn_menu.cpp
        p_ceilng.cpp
        p_doors.cpp
        p_enemy.cpp          
        p_floor.cpp
        p_inter.cpp
        p_lights.cpp
        p_local.cpp          
        p_map.cpp
        p_maputl.cpp
        p_mobj.cpp           
        p_plats.cpp
        p_pspr.cpp           
        p_saveg.cpp
        p_setup.cpp
        p_sight.cpp
        p_spec.cpp
        p_switch.cpp
        p_telept.cpp
        p_tick.cpp
        p_user.cpp
        r_bsp.cpp
        r_data.cpp
        r_draw.cpp
        r_main.cpp
        r_plane.cpp
        r_segs.cpp
        r_things.cpp
        sb_bar.cpp
        sounds.cpp
        s_sound.cpp
    PRIVATE FILE_SET HEADERS FILES
        am_map.hpp
        ct_chat.hpp
        deh_htic.hpp
        doomdata.hpp
        doomdef.hpp
        dstrings.hpp
        info.hpp
        m_random.hpp
        p_action.hpp
        p_enemy.hpp
        p_inter.hpp
        p_local.hpp
        p_mobj.hpp
        p_pspr.hpp
        p_spec.hpp
        r_local.hpp
        sounds.hpp
        s_sound.hpp
    PUBLIC FILE_SET CXX_MODULES FILES
        #m_fixed.cxx
)

target_include_directories(heretic PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(heretic cpp_modules textscreen SDL2::SDL2 SDL2::mixer SDL2::net)

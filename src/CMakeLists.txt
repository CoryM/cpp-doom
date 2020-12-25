foreach(SUBDIR doom setup test)
    add_subdirectory("${SUBDIR}")
endforeach()

include(CheckIPOSupported)
check_ipo_supported(RESULT FoundIPO)

# Common source files used by absolutely everything:

set(COMMON_SOURCE_FILES
    common.cpp             common.hpp
    crispy.cpp             crispy.hpp
    i_main.cpp
    i_system.cpp           i_system.hpp
    m_argv.cpp             m_argv.hpp
    m_misc.cpp             m_misc.hpp)

# Dedicated server (chocolate-server):

set(DEDSERV_FILES
    d_dedicated.cpp
    d_iwad.cpp            d_iwad.hpp
    d_mode.cpp            d_mode.hpp
    deh_str.cpp           deh_str.hpp
    i_timer.cpp           i_timer.hpp
    m_config.cpp          m_config.hpp
    net_common.cpp        net_common.hpp
    net_dedicated.cpp     net_dedicated.hpp
    net_io.cpp            net_io.hpp
    net_packet.cpp        net_packet.hpp
    net_sdl.cpp           net_sdl.hpp
    net_query.cpp         net_query.hpp
    net_server.cpp        net_server.hpp
    net_structrw.cpp      net_structrw.hpp
    z_native.cpp          z_zone.hpp ../utils/lump.hpp)

add_executable("${PROGRAM_PREFIX}server" ${COMMON_SOURCE_FILES} ${DEDSERV_FILES})
target_include_directories("${PROGRAM_PREFIX}server"
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries("${PROGRAM_PREFIX}server" SDL2::SDL2main SDL2::net )
if(FoundIPO)
  set_property(TARGET "${PROGRAM_PREFIX}server" PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

# Source files used by the game binaries (chocolate-doom, etc.)

set(GAME_SOURCE_FILES
    aes_prng.cpp          aes_prng.hpp
    d_event.cpp           d_event.hpp
                          doomkeys.hpp
                          doomtype.hpp
    d_iwad.cpp            d_iwad.hpp
    d_loop.cpp            d_loop.hpp
    d_mode.cpp            d_mode.hpp
                          d_ticcmd.hpp
    deh_str.cpp           deh_str.hpp
    gusconf.cpp           gusconf.hpp
    i_cdmus.cpp           i_cdmus.hpp
    i_endoom.cpp          i_endoom.hpp
    i_glob.cpp            i_glob.hpp
    i_input.cpp           i_input.hpp
    i_joystick.cpp        i_joystick.hpp
                          i_swap.hpp
    i_midipipe.cpp        i_midipipe.hpp
    i_musicpack.cpp
    i_oplmusic.cpp
    i_pcsound.cpp
    i_sdlmusic.cpp
    i_sdlsound.cpp
    i_sound.cpp           i_sound.hpp
    i_timer.cpp           i_timer.hpp
    i_video.cpp           i_video.hpp
    i_videohr.cpp         i_videohr.hpp
    midifile.cpp          midifile.hpp
    mus2mid.cpp           mus2mid.hpp
    m_bbox.cpp            m_bbox.hpp
    m_cheat.cpp           m_cheat.hpp
    m_config.cpp          m_config.hpp
    m_controls.cpp        m_controls.hpp
    m_fixed.cpp           m_fixed.hpp
    net_client.cpp        net_client.hpp
    net_common.cpp        net_common.hpp
    net_dedicated.cpp     net_dedicated.hpp
    net_defs.hpp
    net_gui.cpp           net_gui.hpp
    net_io.cpp            net_io.hpp
    net_loop.cpp          net_loop.hpp
    net_packet.cpp        net_packet.hpp
    net_petname.cpp       net_petname.hpp
    net_query.cpp         net_query.hpp
    net_sdl.cpp           net_sdl.hpp
    net_server.cpp        net_server.hpp
    net_structrw.cpp      net_structrw.hpp
    sha1.cpp              sha1.hpp
    memio.cpp             memio.hpp
    tables.cpp            tables.hpp
    v_diskicon.cpp        v_diskicon.hpp
    v_video.cpp           v_video.hpp
                          v_patch.hpp
    v_trans.cpp           v_trans.hpp
    w_checksum.cpp        w_checksum.hpp
    w_main.cpp            w_main.hpp
    w_wad.cpp             w_wad.hpp
    w_file.cpp            w_file.hpp
    w_file_stdc.cpp
    w_file_posix.cpp
    w_file_win32.cpp
    w_merge.cpp           w_merge.hpp
    z_zone.cpp            z_zone.hpp)

set(GAME_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/../")

set(DEHACKED_SOURCE_FILES
    deh_defs.hpp
    deh_io.cpp            deh_io.hpp
    deh_main.cpp          deh_main.hpp
    deh_mapping.cpp       deh_mapping.hpp
    deh_text.cpp)

# Some games support dehacked patches, some don't:

set(SOURCE_FILES ${COMMON_SOURCE_FILES} ${GAME_SOURCE_FILES})
set(SOURCE_FILES_WITH_DEH ${SOURCE_FILES} ${DEHACKED_SOURCE_FILES})

set(EXTRA_LIBS SDL2::SDL2main SDL2::SDL2 SDL2::mixer SDL2::net fmt::fmt textscreen pcsound opl)
if(SAMPLERATE_FOUND)
    list(APPEND EXTRA_LIBS samplerate::samplerate)
endif()
if(PNG_FOUND)
    list(APPEND EXTRA_LIBS PNG::PNG)
endif()

add_executable("${PROGRAM_PREFIX}doom" ${SOURCE_FILES_WITH_DEH})

target_include_directories("${PROGRAM_PREFIX}doom" PRIVATE ${GAME_INCLUDE_DIRS})
target_link_libraries(
    "${PROGRAM_PREFIX}doom"
    doom
    ${EXTRA_LIBS}
    project_options
)

if(FoundIPO)
  set_property(TARGET "${PROGRAM_PREFIX}doom" PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

# Source files needed for chocolate-setup:

set(SETUP_FILES
    deh_str.cpp           deh_str.hpp
    d_mode.cpp            d_mode.hpp
    d_iwad.cpp            d_iwad.hpp
    i_timer.cpp           i_timer.hpp
    m_config.cpp          m_config.hpp
    m_controls.cpp        m_controls.hpp
    net_io.cpp            net_io.hpp
    net_packet.cpp        net_packet.hpp
    net_petname.cpp       net_petname.hpp
    net_sdl.cpp           net_sdl.hpp
    net_query.cpp         net_query.hpp
    net_structrw.cpp      net_structrw.hpp
    z_native.cpp          z_zone.hpp)

add_executable("${PROGRAM_PREFIX}setup" ${SETUP_FILES} ${COMMON_SOURCE_FILES})

target_include_directories("${PROGRAM_PREFIX}setup"
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries("${PROGRAM_PREFIX}setup" SDL2::SDL2main SDL2::SDL2 SDL2::mixer SDL2::net setup textscreen)

add_executable(midiread midifile.cpp z_native.cpp i_system.cpp m_argv.cpp m_misc.cpp d_iwad.cpp deh_str.cpp m_config.cpp)
target_compile_definitions(midiread PRIVATE "-DTEST")
target_include_directories(midiread PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries(midiread SDL2::SDL2main SDL2::SDL2)

add_executable(mus2mid mus2mid.cpp memio.cpp z_native.cpp i_system.cpp m_argv.cpp m_misc.cpp d_iwad.cpp deh_str.cpp m_config.cpp)
target_compile_definitions(mus2mid PRIVATE "-DSTANDALONE")
target_include_directories(mus2mid PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries(mus2mid SDL2::SDL2main SDL2::SDL2)
if(FoundIPO)
set_property(TARGET mus2mid PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()


#Setup the test executable using Catch2

set(TEST_FILES
    test_main.cpp
    test/test-all.hpp
)
list(REMOVE_ITEM SOURCE_FILES_WITH_DEH i_main.cpp)
add_executable("${PROGRAM_PREFIX}test" ${TEST_FILES} ${SOURCE_FILES_WITH_DEH} )
target_include_directories("${PROGRAM_PREFIX}test" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries("${PROGRAM_PREFIX}test" test doom Catch2::Catch2 ${EXTRA_LIBS})

# Fails due to
#   lto1: note: variable tracking size limit exceeded with ‘-fvar-tracking-assignments’, retrying without
#if(FoundIPO)
#set_property(TARGET "${PROGRAM_PREFIX}test" PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
#endif()

# ------------------- cpp-build -------------------

set(BUILD_INFO_FILES
    build-info/b_main.cpp
    build-info/compiler_feature.hpp
    build-info/compiler_feature.cpp
    build-info/config.hpp
    build-info/config.cpp
    build-info/cxx.hpp
    build-info/cxx.cpp
    build-info/cxx-attributes.cpp
    build-info/cxx-attributes.hpp
    build-info/cxx11.hpp
    build-info/cxx11.cpp
    build-info/cxx14.hpp
    build-info/cxx14.cpp
    build-info/cxx17.hpp
    build-info/cxx17.cpp
    build-info/cxx20.hpp
    build-info/cxx20.cpp
    build-info/cxx23.hpp
    build-info/cxx23.cpp
)

add_executable("${PROGRAM_PREFIX}build" ${BUILD_INFO_FILES})
target_include_directories("${PROGRAM_PREFIX}build" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")

target_link_libraries(
    "${PROGRAM_PREFIX}build"
    ${EXTRA_LIBS}
    project_options
)

if(FoundIPO)
  set_property(TARGET "${PROGRAM_PREFIX}doom" PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

#----------------------
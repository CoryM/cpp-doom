set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.29)
project("Cpp Doom" VERSION 5.7.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Autotools variables
set(top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})
set(top_builddir ${CMAKE_CURRENT_BINARY_DIR})

# AC_INIT variables
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "cpp-doom")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/fabiangreffrath/cpp-doom/issues")

string(REGEX REPLACE " Doom$" "" PACKAGE_SHORTNAME "${PACKAGE_NAME}")
set(PACKAGE_COPYRIGHT "Copyright (C) 1993-2020")
set(PACKAGE_LICENSE "GNU General Public License, version 2")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Any settings that should apply to all targets in this directory and all
# subdirectories should go here.  Use judiciously.
if(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS" "/D_CRT_SECURE_NO_DEPRECATE"
                    "/D_CRT_NONSTDC_NO_DEPRECATE")
else()
    add_compile_options("-Og" "-Wall" "-Wextra" "-fdiagnostics-color=always" "-fpermissive" "-Wfatal-errors"
        "-fsanitize=address" 
        #"-fsanitize=undefined"
        #"-fsanitize=leak" 
        #"-fsanitize=pointer-compare"
        #"-fsanitize=pointer-subtract"
        #"-fno-omit-frame-pointer" 
        #"-fno-optimize-sibling-calls"
        #"-fno-common" 
        #"-fno-omit-frame-pointer"
        )

    add_link_options(
        "-fuse-ld=gold"
        "-fsanitize=address"
        #"-fsanitize=undefined"
        #"-fsanitize=leak" 
        #"-fsanitize=pointer-compare"
        #"-fsanitize=pointer-subtract"
        #"-fno-omit-frame-pointer" 
        #"-fno-optimize-sibling-calls"
        #"-fno-common" 
        #"-fno-omit-frame-pointer"
        )
endif()
add_compile_definitions( -DCRISPY_TRUECOLOR )

find_package(SDL2 2.0.1 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net   REQUIRED)

# Check for libsamplerate.
find_package(samplerate)
if(SAMPLERATE_FOUND)
    set(HAVE_LIBSAMPLERATE TRUE)
endif()

# Check for libpng.
find_package(PNG)
if(PNG_FOUND)
    set(HAVE_LIBPNG TRUE)
endif()

find_package(m)

#include(CheckSymbolExists) # Bugs out on c++ module support
#include(CheckIncludeFile)
#check_symbol_exists(strcasecmp "strings.h" HAVE_DECL_STRCASECMP)
#check_symbol_exists(strncasecmp "strings.h" HAVE_DECL_STRNCASECMP)

set(HAVE_DIRENT_H True)


string(CONCAT WINDOWS_RC_VERSION "${PROJECT_VERSION_MAJOR}, "
    "${PROJECT_VERSION_MINOR}, ${PROJECT_VERSION_PATCH}, 0")

# Without a hyphen. This is used for the bash-completion scripts.
string(TOLOWER "${PACKAGE_SHORTNAME}" PROGRAM_SPREFIX)

# With a hyphen, used almost everywhere else.
set(PROGRAM_PREFIX "${PROGRAM_SPREFIX}-")

configure_file(cmake/config.h.cin config.h)

configure_file(src/resource.rc.in src/resource.rc)
configure_file(src/setup-res.rc.in src/setup-res.rc)
configure_file(src/setup/setup-manifest.xml.in src/setup/setup-manifest.xml)

foreach(SUBDIR textscreen midiproc opl pcsound src)
    add_subdirectory("${SUBDIR}")
endforeach()

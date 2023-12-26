# default.nix
with import <nixpkgs> {};

gcc13Stdenv.mkDerivation {
    name = "cpp-doom"; # Probably put a more meaningful name here

    buildInputs = [ 
        # Debugers
        gdb 
        
        # Build utils
        cmake
        extra-cmake-modules
        ninja
        pkg-config
        
        # Compilers
        #gcc13  # included with gcc13Stdenv
        llvmPackages_17.clang-unwrapped
        
        # Libraries
        SDL2.dev
        SDL2_mixer.dev
        SDL2_net.dev
        #fmt.dev
        #gtest.dev
        libsamplerate.dev
        zlib.dev
        #pngpp
        libpng.dev
    ];

    # Define witch compiler to use
    CXX = "${gcc13}/bin/g++";
    CC = "${gcc13}/bin/gcc";

    # Following should be initilized to CXX and CC by cmake
    #CMAKE_C_COMPILER = "${gcc13}/bin/gcc";
    #CMAKE_CXX_COMPILER = "${gcc13}/bin/g++";

    # Sometimes the PATH needs some help
    #PATH = "${cmake}/bin:${ninja}/bin:${gcc}/bin:$PATH";
}


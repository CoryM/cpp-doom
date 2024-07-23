# default.nix
with import <nixpkgs> {};

let
  unstable = import <nixos-unstable> {};
in 
#gcc14Stdenv.mkDerivation {
llvmPackages_18.libcxxStdenv.mkDerivation {
    name = "cpp-doom"; # Probably put a more meaningful name here

    buildInputs = [ 
        # Debugers
        gdb
        
        # Build utils
        unstable.cmake
        unstable.extra-cmake-modules
        unstable.ninja
        pkg-config
        
        # Compilers
        #gcc14
        llvmPackages_18.libcxxClang
        llvmPackages_18.clangUseLLVM
        
        # Libraries
        SDL2.dev
        SDL2_mixer.dev
        SDL2_net.dev
        libsamplerate.dev
        zlib.dev
        libpng.dev
        glib.dev
        pcre2.dev
        libsndfile.dev
        pulseaudio.dev
        alsa-lib.dev
        jack2.dev

        # Editors and utilities
        unstable.vscode-fhs # Included here so it can find the Standard Libraries

        include-what-you-use
        clang-tools_18
    ];

    shellHook = ''
      #export CXX="${gcc14}/bin/g++"
      #export CC="${gcc14}/bin/gcc"
      export CXX="${llvmPackages_18.libcxxClang}/bin/clang++"
      export CC="${llvmPackages_18.libcxxClang}/bin/clang"
      export ASAN_SYMBOLIZER_PATH="${llvmPackages_18.libcxxClang}/bin/addr2line"
      #export CPLUS_INCLUDE_PATH="${llvmPackages_18.libcxxStdenv}"
    '';
  }

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
        #llvmPackages_17.clang-unwrapped
        
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
        
        # Editors 
	      vscode-fhs # Included here so it can find the Standard Libraries
    ];

    shellHook = ''
      export CXX="${gcc13}/bin/g++"
      export CC="${gcc13}/bin/gcc"
      # ${gcc13} give the wrapper not the nix store path with the includes
      #export CPLUS_INCLUDE_PATH="${gcc13}"
    '';
}


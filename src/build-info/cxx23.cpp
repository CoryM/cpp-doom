#include "cxx23.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const cxx23 = {
    //< Continue to Populate
    COMPILER_FEATURE_ENTRY(__cpp_size_t_suffix) //
};


std::vector<CompilerFeature> const cxx23lib = {
    //< Continue to Populate
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_scoped_enum)  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_stacktrace)      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_stdatomic_h)     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_contains) //
};

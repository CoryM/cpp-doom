#include "cxx.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const cxx = {
    COMPILER_FEATURE_ENTRY(__cplusplus)      // //
    COMPILER_FEATURE_ENTRY(__cpp_exceptions) //
    COMPILER_FEATURE_ENTRY(__cpp_rtti)       //

    COMPILER_FEATURE_ENTRY(__GNUC__)             //
    COMPILER_FEATURE_ENTRY(__GNUC_MINOR__)       //
    COMPILER_FEATURE_ENTRY(__GNUC_PATCHLEVEL__)  //
    COMPILER_FEATURE_ENTRY(__GNUG__)             //
    COMPILER_FEATURE_ENTRY(__clang__)            //
    COMPILER_FEATURE_ENTRY(__clang_major__)      //
    COMPILER_FEATURE_ENTRY(__clang_minor__)      //
    COMPILER_FEATURE_ENTRY(__clang_patchlevel__) //
};
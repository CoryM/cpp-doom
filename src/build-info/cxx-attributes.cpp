#include "cxx-attributes.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const attributes = {
    COMPILER_ATTRIBUTE_ENTRY(carries_dependency) //
    COMPILER_ATTRIBUTE_ENTRY(deprecated)         //
    COMPILER_ATTRIBUTE_ENTRY(fallthrough)        //
    COMPILER_ATTRIBUTE_ENTRY(likely)             //
    COMPILER_ATTRIBUTE_ENTRY(maybe_unused)       //
    COMPILER_ATTRIBUTE_ENTRY(nodiscard)          //
    COMPILER_ATTRIBUTE_ENTRY(noreturn)           //
    COMPILER_ATTRIBUTE_ENTRY(no_unique_address)  //
    COMPILER_ATTRIBUTE_ENTRY(unlikely)           //
};
#include "cxx11.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const cxx11 = {
    COMPILER_FEATURE_ENTRY(__cpp_alias_templates)         //
    COMPILER_FEATURE_ENTRY(__cpp_attributes)              //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)               //
    COMPILER_FEATURE_ENTRY(__cpp_decltype)                //
    COMPILER_FEATURE_ENTRY(__cpp_delegating_constructors) //
    COMPILER_FEATURE_ENTRY(__cpp_inheriting_constructors) //
    COMPILER_FEATURE_ENTRY(__cpp_initializer_lists)       //
    COMPILER_FEATURE_ENTRY(__cpp_lambdas)                 //
    COMPILER_FEATURE_ENTRY(__cpp_nsdmi)                   //
    COMPILER_FEATURE_ENTRY(__cpp_range_based_for)         //
    COMPILER_FEATURE_ENTRY(__cpp_raw_strings)             //
    COMPILER_FEATURE_ENTRY(__cpp_ref_qualifiers)          //
    COMPILER_FEATURE_ENTRY(__cpp_rvalue_references)       //
    COMPILER_FEATURE_ENTRY(__cpp_static_assert)           //
    COMPILER_FEATURE_ENTRY(__cpp_threadsafe_static_init)  //
    COMPILER_FEATURE_ENTRY(__cpp_unicode_characters)      //
    COMPILER_FEATURE_ENTRY(__cpp_unicode_literals)        //
    COMPILER_FEATURE_ENTRY(__cpp_user_defined_literals)   //
    COMPILER_FEATURE_ENTRY(__cpp_variadic_templates)      //
};
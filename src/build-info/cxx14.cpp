#include "cxx14.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const cxx14 = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_nsdmi)       //
    COMPILER_FEATURE_ENTRY(__cpp_binary_literals)       //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)             //
    COMPILER_FEATURE_ENTRY(__cpp_decltype_auto)         //
    COMPILER_FEATURE_ENTRY(__cpp_generic_lambdas)       //
    COMPILER_FEATURE_ENTRY(__cpp_init_captures)         //
    COMPILER_FEATURE_ENTRY(__cpp_return_type_deduction) //
    COMPILER_FEATURE_ENTRY(__cpp_sized_deallocation)    //
    COMPILER_FEATURE_ENTRY(__cpp_variable_templates)    //
};


std::vector<CompilerFeature> const cxx14lib = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono_udls)                  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_complex_udls)                 //
    COMPILER_FEATURE_ENTRY(__cpp_lib_exchange_function)            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_generic_associative_lookup)   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_integer_sequence)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_integral_constant_callable)   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_final)                     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_null_pointer)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_reverse_iterator)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_unique)                  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_null_iterators)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_quoted_string_io)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_result_of_sfinae)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_robust_nonmodifying_seq_ops)  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_timed_mutex)           //
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_udls)                  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_transformation_trait_aliases) //
    COMPILER_FEATURE_ENTRY(__cpp_lib_transparent_operators)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_tuple_element_t)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_tuples_by_type)               //
};

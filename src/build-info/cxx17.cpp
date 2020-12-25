#include "cxx17.hpp"

#include <vector>

#include "compiler_feature.hpp"

std::vector<CompilerFeature> const cxx17 = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_bases)                 //
    COMPILER_FEATURE_ENTRY(__cpp_aligned_new)                     //
    COMPILER_FEATURE_ENTRY(__cpp_capture_star_this)               //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)                       //
    COMPILER_FEATURE_ENTRY(__cpp_deduction_guides)                //
    COMPILER_FEATURE_ENTRY(__cpp_enumerator_attributes)           //
    COMPILER_FEATURE_ENTRY(__cpp_fold_expressions)                //
    COMPILER_FEATURE_ENTRY(__cpp_guaranteed_copy_elision)         //
    COMPILER_FEATURE_ENTRY(__cpp_hex_float)                       //
    COMPILER_FEATURE_ENTRY(__cpp_if_constexpr)                    //
    COMPILER_FEATURE_ENTRY(__cpp_inheriting_constructors)         //
    COMPILER_FEATURE_ENTRY(__cpp_inline_variables)                //
    COMPILER_FEATURE_ENTRY(__cpp_namespace_attributes)            //
    COMPILER_FEATURE_ENTRY(__cpp_noexcept_function_type)          //
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_args)           //
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_parameter_auto) //
    COMPILER_FEATURE_ENTRY(__cpp_range_based_for)                 //
    COMPILER_FEATURE_ENTRY(__cpp_static_assert)                   //
    COMPILER_FEATURE_ENTRY(__cpp_structured_bindings)             //
    COMPILER_FEATURE_ENTRY(__cpp_template_template_args)          //
    COMPILER_FEATURE_ENTRY(__cpp_variadic_using)                  //
};


std::vector<CompilerFeature> const cxx17lib = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_addressof_constexpr)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_allocator_traits_is_always_equal)  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_any)                               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_apply)                             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_array_constexpr)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_as_const)                          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_is_always_lock_free)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_bool_constant)                     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_boyer_moore_searcher)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_byte)                              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono)                            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_clamp)                             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_enable_shared_from_this)           //
    COMPILER_FEATURE_ENTRY(__cpp_lib_execution)                         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_filesystem)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_gcd_lcm)                           //
    COMPILER_FEATURE_ENTRY(__cpp_lib_hardware_interference_size)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_has_unique_object_representations) //
    COMPILER_FEATURE_ENTRY(__cpp_lib_hypot)                             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_incomplete_container_elements)     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_invoke)                            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_aggregate)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_invocable)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_swappable)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_launder)                           //
    COMPILER_FEATURE_ENTRY(__cpp_lib_logical_traits)                    //
    COMPILER_FEATURE_ENTRY(__cpp_lib_make_from_tuple)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_map_try_emplace)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_math_special_functions)            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_memory_resource)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_node_extract)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_nonmember_container_access)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_not_fn)                            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_optional)                          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_parallel_algorithm)                //
    COMPILER_FEATURE_ENTRY(__cpp_lib_raw_memory_algorithms)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_sample)                            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_scoped_lock)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_mutex)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_arrays)                 //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_weak_type)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_view)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_chars)                          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_transparent_operators)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_type_trait_variable_templates)     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_uncaught_exceptions)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_unordered_map_try_emplace)         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_variant)                           //
    COMPILER_FEATURE_ENTRY(__cpp_lib_void_t)                            //
};

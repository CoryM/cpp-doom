#include "cxx20.hpp"

#include <vector>

#include "compiler_feature.hpp"


std::vector<CompilerFeature> const cxx20 = {
    COMPILER_FEATURE_ENTRY(__cpp_aggregate_paren_init)      //
    COMPILER_FEATURE_ENTRY(__cpp_char8_t)                   //
    COMPILER_FEATURE_ENTRY(__cpp_concepts)                  //
    COMPILER_FEATURE_ENTRY(__cpp_conditional_explicit)      //
    COMPILER_FEATURE_ENTRY(__cpp_consteval)                 //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr)                 //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr_dynamic_alloc)   //
    COMPILER_FEATURE_ENTRY(__cpp_constexpr_in_decltype)     //
    COMPILER_FEATURE_ENTRY(__cpp_constinit)                 //
    COMPILER_FEATURE_ENTRY(__cpp_deduction_guides)          //
    COMPILER_FEATURE_ENTRY(__cpp_designated_initializers)   //
    COMPILER_FEATURE_ENTRY(__cpp_generic_lambdas)           //
    COMPILER_FEATURE_ENTRY(__cpp_impl_coroutine)            //
    COMPILER_FEATURE_ENTRY(__cpp_impl_destroying_delete)    //
    COMPILER_FEATURE_ENTRY(__cpp_impl_three_way_comparison) //
    COMPILER_FEATURE_ENTRY(__cpp_init_captures)             //
    COMPILER_FEATURE_ENTRY(__cpp_modules)                   //
    COMPILER_FEATURE_ENTRY(__cpp_nontype_template_args)     //
    COMPILER_FEATURE_ENTRY(__cpp_using_enum)                //
};


std::vector<CompilerFeature> const cxx20lib = {
    COMPILER_FEATURE_ENTRY(__cpp_lib_array_constexpr)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_assume_aligned)                //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_flag_test)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_float)                  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_lock_free_type_aliases) //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_ref)                    //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_shared_ptr)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_value_initialization)   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_atomic_wait)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_barrier)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_bind_front)                    //
    COMPILER_FEATURE_ENTRY(__cpp_lib_bit_cast)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_bitops)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_bounded_array_traits)          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_char8_t)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_chrono)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_concepts)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_algorithms)          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_complex)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_dynamic_alloc)       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_functional)          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_iterator)            //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_memory)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_numeric)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_string)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_string_view)         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_tuple)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_utility)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_constexpr_vector)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_coroutine)                     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_destroying_delete)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_endian)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_erase_if)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_execution)                     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_format)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_generic_unordered_lookup)      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_int_pow2)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_integer_comparison_functions)  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_interpolate)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_constant_evaluated)         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_layout_compatible)          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_nothrow_convertible)        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_is_pointer_interconvertible)   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_jthread)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_latch)                         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_list_remove_return_type)       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_math_constants)                //
    COMPILER_FEATURE_ENTRY(__cpp_lib_polymorphic_allocator)         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_ranges)                        //
    COMPILER_FEATURE_ENTRY(__cpp_lib_remove_cvref)                  //
    COMPILER_FEATURE_ENTRY(__cpp_lib_semaphore)                     //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shared_ptr_arrays)             //
    COMPILER_FEATURE_ENTRY(__cpp_lib_shift)                         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_smart_ptr_for_overwrite)       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_source_location)               //
    COMPILER_FEATURE_ENTRY(__cpp_lib_span)                          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_ssize)                         //
    COMPILER_FEATURE_ENTRY(__cpp_lib_starts_ends_with)              //
    COMPILER_FEATURE_ENTRY(__cpp_lib_string_view)                   //
    COMPILER_FEATURE_ENTRY(__cpp_lib_syncbuf)                       //
    COMPILER_FEATURE_ENTRY(__cpp_lib_three_way_comparison)          //
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_address)                    //
    COMPILER_FEATURE_ENTRY(__cpp_lib_to_array)                      //
    COMPILER_FEATURE_ENTRY(__cpp_lib_type_identity)                 //
    COMPILER_FEATURE_ENTRY(__cpp_lib_unwrap_ref)                    //
};

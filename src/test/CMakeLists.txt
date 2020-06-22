add_library(test STATIC
            test-all.hpp
            d_iwad_test.hpp
            )
set_target_properties(test PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(test PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(test GTest::gtest)

include(CheckIPOSupported)
check_ipo_supported(RESULT FoundIPO)

if(FoundIPO)
set_property(TARGET test PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

add_library(test STATIC
            test1.cpp)

target_include_directories(test PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(test GTest::gtest)

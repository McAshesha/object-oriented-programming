cmake_minimum_required(VERSION 4.0)
project(object_oriented_programming)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== lib with core classes =====
add_library(image STATIC
        Image.cpp
        Range.cpp
)
target_include_directories(image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ===== CLI tool =====
add_executable(imgtool
        main.cpp
        ppm_io.cpp
        ops.cpp
)
target_link_libraries(imgtool PRIVATE image)

# ===== GoogleTest =====
enable_testing()
find_package(GTest REQUIRED)
add_executable(image_tests tests_gtest.cpp)
target_link_libraries(image_tests PRIVATE image GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(image_tests)

# ===== Gate build of imgtool by tests =====
add_custom_target(verify_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS image_tests
        USES_TERMINAL
)

add_dependencies(imgtool verify_tests)

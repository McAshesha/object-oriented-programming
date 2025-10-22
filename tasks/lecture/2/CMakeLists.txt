cmake_minimum_required(VERSION 4.0)
project(object_oriented_programming LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== Core library (to share between exe and tests) =====
add_library(pd3core STATIC
    src/PayoffMatrix.cpp
    src/GameRunner.cpp
    src/StrategyFactory.cpp
    src/strategies/BuiltinStrategies.cpp
)
target_include_directories(pd3core PUBLIC include src)
if(UNIX)
    target_link_libraries(pd3core PUBLIC dl)
endif()

# ===== Executable =====
add_executable(pd3
    src/main.cpp
)
target_link_libraries(pd3 PRIVATE pd3core)

# ===== Sample plugin =====
add_library(Strategy_AdaptiveGrim SHARED
    plugins/AdaptiveGrim/plugin.cpp
)
target_include_directories(Strategy_AdaptiveGrim PRIVATE include)
set_target_properties(Strategy_AdaptiveGrim PROPERTIES
    OUTPUT_NAME "Strategy_AdaptiveGrim"
)
if(UNIX)
    set_target_properties(Strategy_AdaptiveGrim PROPERTIES PREFIX "lib")
endif()

# ===== GoogleTest =====
enable_testing()
find_package(GTest REQUIRED)

add_executable(pd3_tests
    tests/pd3_tests.cpp
)
target_link_libraries(pd3_tests PRIVATE pd3core GTest::gtest_main)

# Provide build dir as PLUGINS_DIR for the test to locate the built plugin
target_compile_definitions(pd3_tests PRIVATE PLUGINS_DIR="$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>")

# Ensure plugin is built before tests run
add_dependencies(pd3_tests Strategy_AdaptiveGrim)

include(GoogleTest)
gtest_discover_tests(pd3_tests)

# ===== Gate build of pd3 by tests (optional, like your pattern) =====
add_custom_target(verify_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS pd3_tests
    USES_TERMINAL
)
add_dependencies(pd3 verify_tests)

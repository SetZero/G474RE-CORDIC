cmake_minimum_required(VERSION 3.7)
project(G474RE-CORDIC-TESTS ASM C CXX)

set(REGISTER_TEST_SOURCES
        register_tests.cpp
        ../src/hal/register.h)
add_executable(register_tests register_tests.cpp)
target_include_directories(register_tests PUBLIC ../src)
target_compile_options(register_tests PUBLIC -Werror -Wall -Wextra -Wpedantic -Wshadow)
target_compile_options(register_tests PUBLIC -fconcepts)
set_property(TARGET register_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET register_tests PROPERTY C_STANDARD 11)

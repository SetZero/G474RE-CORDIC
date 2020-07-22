cmake_minimum_required(VERSION 3.7)
project(G474RE-CORDIC-TESTS ASM C CXX)

enable_testing()
set(REGISTER_TEST_SOURCES
        register_tests.cpp
        ../src/hal/register.h)
add_executable(register_tests ${REGISTER_TEST_SOURCES})
add_test(NAME register_tests_exe COMMAND register_tests)
target_include_directories(register_tests PUBLIC ../src)
target_compile_options(register_tests PUBLIC -Werror -Wall -Wextra -Wpedantic -Wshadow)
target_compile_options(register_tests PUBLIC -fconcepts)
set_property(TARGET register_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET register_tests PROPERTY C_STANDARD 11)

set(UTILS_TEST_SOURCES
    utils_tests.cpp
    ../src/utils.h)

add_executable(utils_tests ${UTILS_TEST_SOURCES})
add_test(NAME utils_tests_exe COMMAND utils_tests)
target_include_directories(utils_tests PUBLIC ../src)
target_compile_options(utils_tests PUBLIC -Werror -Wall -Wextra -Wpedantic -Wshadow)
target_compile_options(utils_tests PUBLIC -fconcepts)
set_property(TARGET utils_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET utils_tests PROPERTY C_STANDARD 11)

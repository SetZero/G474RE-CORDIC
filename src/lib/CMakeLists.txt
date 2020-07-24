option(USE_HAL_DRIVER "Enable the use of the hal driver" off)

add_library(hal_lib INTERFACE)
target_include_directories(hal_lib INTERFACE .)

if (USE_HAL_DRIVER)
    target_compile_definitions(${PROJECT_NAME}.elf INTERFACE USE_HAL_DRIVER)
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
    target_compile_options(${PROJECT_NAME}.elf INTERFACE -fconcepts)
endif()

target_compile_features(hal_lib INTERFACE cxx_std_20 c_std_11)

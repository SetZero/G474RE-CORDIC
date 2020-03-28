set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# specify cross compilers and tools
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_LINKER arm-none-eabi-ld)
set(SIZE arm-none-eabi-size)

option(USE_HAL_DRIVER "Enable the use of the hal driver" on)
option(USE_HARDWARE_FLOAT "Enable the use of hardware floating point operations" off)

set(LINKER_SCRIPT STM32G474RETx_FLASH.ld)
set(SOURCES
    startup/startup_stm32g474xx.s
    src/main.cpp)

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_compile_definitions(${PROJECT_NAME}.elf PUBLIC STM32G474xx)

if (USE_HAL_DRIVER)
    target_compile_definitions(${PROJECT_NAME}.elf PUBLIC USE_HAL_DRIVER)
endif()

if (USE_HARDWARE_FLOAT)
    #Uncomment for software floating point
    #SET(FPU_FLAGS "-mfloat-abi=soft")
else()
    #Uncomment for hardware floating point
    #SET(FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")
    #add_definitions(-DARM_MATH_CM4 -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1)
endif()

target_compile_options(${PROJECT_NAME}.elf PUBLIC
    -mcpu=cortex-m4 ${FPU_FLAGS} -mthumb -mthumb-interwork -ffunction-sections -fdata-sections
    -fno-common -fmessage-length=0 -specs=nosys.specs -specs=nano.specs
    -Wdouble-promotion -fstack-protector-strong)

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    target_compile_options(${PROJECT_NAME}.elf PUBLIC -g)
endif()

target_compile_options(${PROJECT_NAME}.elf PUBLIC -fno-exceptions)
target_compile_options(${PROJECT_NAME}.elf PUBLIC
    -Werror -Wall -Wextra -Wpedantic -Wshadow)
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY CMAKE_C_STANDARD 11)
target_link_options(${PROJECT_NAME}.elf PUBLIC
    -Wl,-gc-sections,--print-memory-usage -T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT})
target_link_options(${PROJECT_NAME}.elf PUBLIC
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
target_link_options(${PROJECT_NAME}.elf PUBLIC -static -static-libgcc -static-libstdc++)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

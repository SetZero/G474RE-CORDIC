#THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!
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

set(LINKER_SCRIPT ./STM32G474RETx_FLASH.ld)

#Uncomment for hardware floating point
#SET(FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")
#add_definitions(-DARM_MATH_CM4 -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1)

#Uncomment for software floating point
#SET(FPU_FLAGS "-mfloat-abi=soft")

set(COMMON_FLAGS
    "-mcpu=cortex-m4 ${FPU_FLAGS} -mthumb -mthumb-interwork -ffunction-sections -fdata-sections \
    -g -fno-common -fmessage-length=0 -nostartfiles -specs=nosys.specs -specs=nano.specs \
    -Wdouble-promotion -fstack-protector-strong -Wall -Wextra -Wpedantic -Wshadow")

set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=c++17")
set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-gc-sections,--print-memory-usage -T ${LINKER_SCRIPT}")


set(SOURCES
    startup/startup_stm32g474xx.s
    src/main.cpp)

#include_directories(Inc Drivers/STM32G4xx_HAL_Driver/Inc Drivers/STM32G4xx_HAL_Driver/Inc/Legacy Drivers/CMSIS/Device/ST/STM32G4xx/Include Drivers/CMSIS/Include)

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
target_compile_definitions(${PROJECT_NAME}.elf PUBLIC USE_HAL_DRIVER)
target_compile_definitions(${PROJECT_NAME}.elf PUBLIC STM32G474xx)
#add_definitions(-DARM_MATH_CM4 -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1)

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map")

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

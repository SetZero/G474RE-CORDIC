option(USE_HAL_DRIVER "Enable the use of the hal driver" off)
option(USE_HARDWARE_FLOAT "Enable the use of hardware floating point operations" off)

set(LINKER_SCRIPT STM32G474RETx_FLASH.ld)
set(SOURCES
    startup/startup_stm32g474xx.s
    src/main.cpp src/hal/stm32/stm32g4.h src/hal/register.h)

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_compile_definitions(${PROJECT_NAME}.elf PUBLIC STM32G474xx)

if (USE_HAL_DRIVER)
    target_compile_definitions(${PROJECT_NAME}.elf PUBLIC USE_HAL_DRIVER)
endif()

target_compile_options(${PROJECT_NAME}.elf PUBLIC
    -mcpu=cortex-m4 ${FPU_FLAGS} -mthumb -mthumb-interwork -ffunction-sections -fdata-sections
    -fno-common -fmessage-length=0 -Wdouble-promotion -fstack-protector-strong)

target_compile_options(${PROJECT_NAME}.elf PUBLIC -fno-exceptions)
target_compile_options(${PROJECT_NAME}.elf PUBLIC
    -Werror -Wall -Wextra -Wpedantic -Wshadow)
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY C_STANDARD 11)

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    target_compile_options(${PROJECT_NAME}.elf PUBLIC -g)
    message("Build Debug")
endif()

if (USE_HARDWARE_FLOAT)
    target_compile_options(${PROJECT_NAME}.elf PUBLIC -mfloat-abi=hard -mfpu=fpv4-sp-d16)
    target_compile_definitions(${PROJECT_NAME}.elf PUBLIC ARM_MATH_CM4 ARM_MATH_MATRIX_CHECK ARM_MATH_ROUNDING __FPU_PRESENT=1)
else()
    target_compile_options(${PROJECT_NAME}.elf PUBLIC -mfloat-abi=soft)
endif()

set_target_properties(
  ${PROJECT_NAME}.elf
  PROPERTIES
  LINK_FLAGS
  "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT} \
   -mthumb \
   -mcpu=cortex-m4 \
   -Wl,--gc-sections,--print-memory-usage \
   -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map")

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

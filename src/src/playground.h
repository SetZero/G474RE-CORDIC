//
// Created by Sebastian on 28.07.2020.
//

#pragma once
#include <cstdint>

#include "hal/cordic.h"
#include "hal/cordic_types.h"
#include "hal/gpio.h"
#include "hal/stm32/stm32g4.h"
#include "hal/stm32/stm32g4/stm32g474re.h"
#include "hal/uart.h"

static constexpr auto RCC_BASE = 0x40021000;
static constexpr inline auto GPIO_A_BASE [[gnu::unused]] = 0x48000000;
static constexpr inline auto GPIO_B_BASE [[gnu::unused]] = 0x48000400;
static constexpr inline auto GPIO_C_BASE [[gnu::unused]] = 0x48000800;
static constexpr inline auto GPIO_D_BASE [[gnu::unused]] = 0x48000C00;
static constexpr inline auto GPIO_E_BASE [[gnu::unused]] = 0x48001000;
static constexpr inline auto GPIO_F_BASE [[gnu::unused]] = 0x48001400;
static constexpr inline auto GPIO_G_BASE [[gnu::unused]] = 0x48001800; /* peripherals */
static constexpr inline auto RCC_AHB1ENR [[gnu::unused]] = 0x48;
static constexpr inline auto RCC_AHB2ENR = 0x4C;
static constexpr inline auto RCC_APB1ENR1 [[gnu::unused]] = 0x58;
static constexpr inline auto RCC_APB2ENR [[gnu::unused]] = 0x60;
static constexpr inline auto GPIO_X_MODER = 0x00;
static constexpr inline auto GPIO_X_OTYPER = 0x04;
static constexpr inline auto GPIO_X_OSPEEDR = 0x08;
static constexpr inline auto GPIO_X_ODER [[gnu::unused]] = 0x14;
static constexpr inline auto GPIO_X_BSRR = 0x18;
static constexpr inline auto GPIO_X_AFRL = 0x20;
static constexpr inline auto GPIO_X_AFRH = 0x24;                      /* CORDIC */
static constexpr inline auto CORDIC_BASE [[gnu::unused]] = 0x40020C00; /* TIMER */
static constexpr inline auto TIM2_BASE [[gnu::unused]] = 0x40000000;
static constexpr inline auto TIMER_CR1 [[gnu::unused]] = 0x000;
static constexpr inline auto TIMER_CCMR [[gnu::unused]] = 0x018;
static constexpr inline auto TIMER_CCER [[gnu::unused]] = 0x020;
static constexpr inline auto TIMER_PRESCALER [[gnu::unused]] = 0x028;
static constexpr inline auto TIMER_ARR [[gnu::unused]] = 0x02C;
static constexpr inline auto TIMER_CCR [[gnu::unused]] = 0x034; /* LPUART */
static constexpr inline auto LPUART_BASE [[gnu::unused]] = 0x40008000;
static constexpr inline auto LPUART_CR1 [[gnu::unused]] = 0x00;
static constexpr inline auto LPUART_CR2 [[gnu::unused]] = 0x04;
static constexpr inline auto LPUART_CR3 [[gnu::unused]] = 0x08;
static constexpr inline auto LPUART_BRR [[gnu::unused]] = 0x0C;
static constexpr inline auto LPUART_ISR [[gnu::unused]] = 0x1C;
static constexpr inline auto LPUART_RDR [[gnu::unused]] = 0x24;
static constexpr inline auto LPUART_TDR [[gnu::unused]] = 0x28;
static constexpr inline auto LPUART1SEL [[gnu::unused]] = 0x88; /* UART */
static constexpr inline auto UART_BASE [[gnu::unused]] = 0x40013800;
static constexpr inline auto UART2_BASE [[gnu::unused]] = 0x40004400;
static constexpr inline auto UART_CR1 [[gnu::unused]] = 0x00;
static constexpr inline auto UART_CR2 [[gnu::unused]] = 0x04;
static constexpr inline auto UART_CR3 [[gnu::unused]] = 0x08;
static constexpr inline auto UART_BRR [[gnu::unused]] = 0x0C;
static constexpr inline auto UART_TDR [[gnu::unused]] = 0x28;


/***** GPIO ******/
enum class GPIO_MODES : uint32_t {
    INPUT = 0b0u,
    GPO = 0b1u,
    ALT = 0b10,
    ANALOG = 0b11,
};
enum class PA5_AF_L : uint32_t {
    TIM2_CH1 = (0b0001u << 20u),
    TIM2_ETR = (0b0010u << 20u),
    SPI1_SCK = (0b0101u << 20u),
    UCPD1_FRSTX = (0b1110u << 20u),
    EVENT_OUT = (0b1111u << 20u),
};

/***** TIMER ******/
enum class CR1_DIR : uint32_t {
    DIR_UP = (0u << 4u),
    DIR_DOWN = (1u << 4u),
};
enum class CR1_CLKDIV : uint32_t {
    DIV1 = (0u << 8u),
    DIV2 = (1u << 8u),
    DIV4 = (2u << 8u),
};
enum class CR1_EN : uint32_t {
    ENABLE_TIMER = (1u << 0u),
};
enum class CR1_ARPE : uint32_t {
    BUFFER_ARR = (1u << 7u),
};
enum class CCMR_OC1M : uint32_t {
    PWM_MODE_1 = (0b0110u << 4u),
    PWM_MODE_2 = (0b0111u << 4u),
};

/***** LPUART *****/
enum class UARTCLK : uint32_t { PCLK, SYSCLK, HSI16, LSE }; /* Utils */
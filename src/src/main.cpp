/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 *
 *
 ******************************************************************************
 */

/* Includes */
#include "main.h"

#include <cstdint>
//#include <concepts>
#include "hal/cordic.h"
#include "hal/cordic_types.h"
#include "hal/stm32/stm32g4.h"

/* REGISTER BASE ADDRESSES */

static constexpr auto RCC_BASE = 0x40021000;

static constexpr inline auto GPIO_A_BASE [[gnu::unused]] = 0x48000000;
static constexpr inline auto GPIO_B_BASE [[gnu::unused]] = 0x48000400;
static constexpr inline auto GPIO_C_BASE [[gnu::unused]] = 0x48000800;
static constexpr inline auto GPIO_D_BASE [[gnu::unused]] = 0x48000C00;
static constexpr inline auto GPIO_E_BASE [[gnu::unused]] = 0x48001000;
static constexpr inline auto GPIO_F_BASE [[gnu::unused]] = 0x48001400;
static constexpr inline auto GPIO_G_BASE [[gnu::unused]] = 0x48001800;

/* peripherals */
static constexpr inline auto RCC_AHB1ENR [[gnu::unused]] = 0x48;
static constexpr inline auto RCC_AHB2ENR = 0x4C;
static constexpr inline auto RCC_APB1ENR1 [[gnu::unused]] = 0x58;
static constexpr inline auto RCC_APB2ENR [[gnu::unused]] = 0x60;

/* GPIO */

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

static constexpr inline auto GPIO_X_MODER = 0x00;
static constexpr inline auto GPIO_X_OTYPER = 0x04;
static constexpr inline auto GPIO_X_OSPEEDR = 0x08;
static constexpr inline auto GPIO_X_ODER [[gnu::unused]] = 0x14;
static constexpr inline auto GPIO_X_BSRR = 0x18;
static constexpr inline auto GPIO_X_AFRL = 0x20;
static constexpr inline auto GPIO_X_AFRH = 0x24;

/* CORDIC */
static constexpr inline auto CORDIC_BASE [[gnu::unused]] = 0x40020C00;

/* TIMER */
static constexpr inline auto TIM2_BASE [[gnu::unused]] = 0x40000000;
static constexpr inline auto TIMER_CR1 [[gnu::unused]] = 0x000;
static constexpr inline auto TIMER_CCMR [[gnu::unused]] = 0x018;
static constexpr inline auto TIMER_CCER [[gnu::unused]] = 0x020;
static constexpr inline auto TIMER_PRESCALER [[gnu::unused]] = 0x028;
static constexpr inline auto TIMER_ARR[[gnu::unused]] = 0x02C;
static constexpr inline auto TIMER_CCR[[gnu::unused]] = 0x034;

/* UART */
static constexpr inline auto LPUART1SEL [[gnu::unused]]  = 0x88;

/* Utils */

[[nodiscard]] inline uint32_t &memory(const uint32_t loc) {
    return *reinterpret_cast<uint32_t*>(loc);
}

void delay_ms(uint32_t n) {
    for (; n > 0; n--)
        for (uint32_t i = 0; i < 3195; i++) asm("nop");
}


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

void init_led() {
    HAL::address<HAL::STM::peripherals::AHBENR, 0>()->ahb2.add<HAL::STM::peripherals::AHBENR::AHB2ENR::GPIOA>();
    memory(GPIO_A_BASE + GPIO_X_AFRL) |= static_cast<uint32_t >(PA5_AF_L::TIM2_CH1);
    memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(0b11u << (5 * 2u));
    memory(GPIO_A_BASE + GPIO_X_MODER) |= (static_cast<uint32_t >(GPIO_MODES::ALT) << (5 * 2u));
    memory(GPIO_A_BASE + GPIO_X_OTYPER) &= ~(1u << 5u); // Output push-pull
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) &= ~(11u << 10u); //clear speed
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) |= (10u << 10u); // HIGH Speed
}

void init_timer() {
    HAL::address<HAL::STM::peripherals::APBENR, 0>()->apb11.add<HAL::STM::peripherals::APBENR::APB1ENR1::TIM2EN>();
    memory(TIM2_BASE + TIMER_PRESCALER) = 10;       // 16.000.000 / 10 = 1.6Mhz
    memory(TIM2_BASE + TIMER_ARR) = 26667;         // 1.6Mhz / 26667 = ca. 60Hz
    memory(TIM2_BASE + TIMER_CCR) = 8889;          // pulse width 8889/26667 == 1/3 of period
    memory(TIM2_BASE + TIMER_CCER) = (1u << 0u);   // CC1E

    memory(TIM2_BASE + TIMER_CR1) |= static_cast<uint32_t >(CR1_DIR::DIR_UP)
                                     | static_cast<uint32_t >(CR1_CLKDIV::DIV1)
                                     | static_cast<uint32_t >(CR1_EN::ENABLE_TIMER)
                                     | static_cast<uint32_t >(CR1_ARPE::BUFFER_ARR);
}

void init_pwm() {
    memory(TIM2_BASE + TIMER_CCMR) |= static_cast<uint32_t >(CCMR_OC1M::PWM_MODE_1) | (1u << 11u); // PWM Mode 1 + OC2PE enable
}

/****************/

/***** UART *****/

enum class UARTCLK : uint32_t {
   PCLK,
   SYSCLK,
   HSI16,
   LSE
};

void init_uart() {
    HAL::address<HAL::STM::peripherals::APBENR, 0>()->apb12.add<HAL::STM::peripherals::APBENR::APB1ENR2::LPUART1EN>();
    memory(RCC_BASE + LPUART1SEL) = static_cast<uint32_t >(UARTCLK::PCLK);
}

/****************/

/**
 * @brief  The application entry point.
 * @retval int
 */
int main() {
    // SystemClock_Config();
    // memory(RCC_BASE + RCC_AHB2ENR) |= 1u;
    //auto &csr_reg [[gnu::unused]] = HAL::address<HAL::STM::peripherals::AHBENR, 0>()->ahb1;
    HAL::address<HAL::STM::peripherals::AHBENR, 0>()->ahb1.add<HAL::STM::peripherals::AHBENR::AHB1ENR::CORDIC>();
    HAL::address<HAL::STM::peripherals::AHBENR, 0>()->ahb2.add<HAL::STM::peripherals::AHBENR::AHB2ENR::GPIOA>();

    init_led();
    init_timer();
    init_pwm();

    using namespace CordicHal;

    //using cc = cordic_config<precision::q1_31>;

    //cordic c{HAL::address<HAL::STM::peripherals::CORDIC, 0>()};

    //memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(0b11u << (5 * 2u));
    //memory(GPIO_A_BASE + GPIO_X_MODER) |= (0b1u << (5 * 2u));

    //operation<cc, operation_type::single, functions::cosine> op;

    //int16_t deg = 0;

    while (true) {
        memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << 5u);
        delay_ms(500);
        memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << (5u + 16));
        delay_ms(500);
        /*int16_t rdeg = deg - 180;
        op.arg1(angle<precision::q1_31>{degrees{rdeg}});
        auto result = c.calculate(op);

        memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << 5u);
        // memory(GPIO_A_BASE + GPIO_X_ODER) |= (1u << 5u);
        delay_ms(500);
        // memory(GPIO_A_BASE + GPIO_X_ODER) &= ~(1u << 5u);
        // memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << (5u + 16));
        q1_31 q [[gnu::unused]] = result.result();

        delay_ms(static_cast<uint32_t>((static_cast<float>(q) + 1) * 500));
        deg = (deg + 1) % 360;*/
    }
}

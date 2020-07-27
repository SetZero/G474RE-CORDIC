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
#include "hal/gpio.h"
#include "hal/stm32/stm32g4.h"
#include "hal/stm32/stm32g4/stm32g474re.h"
#include "hal/uart.h"

namespace mcu_ns = hal::stm::stm32g4;
using used_mcu = hal::stm::stm32g4::g474re;
namespace gpio_values = hal::periphery::gpio_values;

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
static constexpr inline auto TIMER_ARR [[gnu::unused]] = 0x02C;
static constexpr inline auto TIMER_CCR [[gnu::unused]] = 0x034;

/* LPUART */
static constexpr inline auto LPUART_BASE [[gnu::unused]] = 0x40008000;
static constexpr inline auto LPUART_CR1 [[gnu::unused]] = 0x00;
static constexpr inline auto LPUART_CR2 [[gnu::unused]] = 0x04;
static constexpr inline auto LPUART_CR3 [[gnu::unused]] = 0x08;
static constexpr inline auto LPUART_BRR [[gnu::unused]] = 0x0C;
static constexpr inline auto LPUART_ISR [[gnu::unused]] = 0x1C;
static constexpr inline auto LPUART_RDR [[gnu::unused]] = 0x24;
static constexpr inline auto LPUART_TDR [[gnu::unused]] = 0x28;
static constexpr inline auto LPUART1SEL [[gnu::unused]] = 0x88;

/* UART */
static constexpr inline auto UART_BASE [[gnu::unused]] = 0x40013800;
static constexpr inline auto UART2_BASE [[gnu::unused]] = 0x40004400;
static constexpr inline auto UART_CR1 [[gnu::unused]] = 0x00;
static constexpr inline auto UART_CR2 [[gnu::unused]] = 0x04;
static constexpr inline auto UART_CR3 [[gnu::unused]] = 0x08;
static constexpr inline auto UART_BRR [[gnu::unused]] = 0x0C;
static constexpr inline auto UART_TDR [[gnu::unused]] = 0x28;

/* Utils */

[[nodiscard]] inline uint32_t &memory(const uint32_t loc) { return *reinterpret_cast<uint32_t *>(loc); }

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
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();
    memory(GPIO_A_BASE + GPIO_X_AFRL) |= static_cast<uint32_t>(PA5_AF_L::TIM2_CH1);
    // memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(0b11u << (5 * 2u));
    hal::address<hal::stm::stm32g4::mcu_info::GPIO, hal::stm::stm32g4::A>()->moder.clear<5>();
    hal::address<hal::stm::stm32g4::mcu_info::GPIO, hal::stm::stm32g4::A>()
        ->moder.add<hal::stm::stm32g4::mcu_info::GPIO::MODER::ALTERNATIVE_FUNCTION, 5>();
    // memory(GPIO_A_BASE + GPIO_X_MODER) |= (static_cast<uint32_t>(GPIO_MODES::ALT) << (5 * 2u));
    memory(GPIO_A_BASE + GPIO_X_OTYPER) &= ~(1u << 5u);     // Output push-pull
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) &= ~(11u << 10u);  // clear speed
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) |= (10u << 10u);   // HIGH Speed
}

void init_timer() {
    hal::address<hal::stm::stm32g4::mcu_info::APBENR, 0>()
        ->apb11.add<hal::stm::stm32g4::mcu_info::APBENR::APB1ENR1::TIM2EN>();
    memory(TIM2_BASE + TIMER_PRESCALER) = 5;      // 16.000.000 / 10 = 1.6Mhz
    memory(TIM2_BASE + TIMER_ARR) = 26667;        // 1.6Mhz / 26667 = ca. 60Hz
    memory(TIM2_BASE + TIMER_CCR) = 8889;         // pulse width 8889/26667 == 1/3 of period
    memory(TIM2_BASE + TIMER_CCER) = (1u << 0u);  // CC1E

    memory(TIM2_BASE + TIMER_CR1) |= static_cast<uint32_t>(CR1_DIR::DIR_UP) | static_cast<uint32_t>(CR1_CLKDIV::DIV1) |
                                     static_cast<uint32_t>(CR1_EN::ENABLE_TIMER) |
                                     static_cast<uint32_t>(CR1_ARPE::BUFFER_ARR);
}

void init_pwm() {
    memory(TIM2_BASE + TIMER_CCMR) |=
        static_cast<uint32_t>(CCMR_OC1M::PWM_MODE_1) | (1u << 11u);  // PWM Mode 1 + OC2PE enable
}

/****************/

/***** LPUART *****/

enum class UARTCLK : uint32_t { PCLK, SYSCLK, HSI16, LSE };

void init_lpuart_pin() {
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();
    // Make GPIOA Pin 2,3 (PA2, PA3) alternate-function output
    memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(1111u << 4u);
    memory(GPIO_A_BASE + GPIO_X_MODER) |= (static_cast<uint32_t>(GPIO_MODES::ALT) << (2 * 2u));
    memory(GPIO_A_BASE + GPIO_X_MODER) |= (static_cast<uint32_t>(GPIO_MODES::ALT) << (3 * 2u));
    memory(GPIO_A_BASE + GPIO_X_AFRL) &= ~(0xFFu << 8u);
    memory(GPIO_A_BASE + GPIO_X_AFRL) |= (0b1100u << 8u);   // LPUART PA2
    memory(GPIO_A_BASE + GPIO_X_AFRL) |= (0b1100u << 12u);  // LPUART PA3

    memory(GPIO_A_BASE + GPIO_X_OTYPER) &= ~(1u << 2u);    // Output push-pull
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) &= ~(11u << 4u);  // clear speed
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) |= (11u << 4u);   // very HIGH Speed

    memory(GPIO_A_BASE + GPIO_X_OTYPER) &= ~(1u << 3u);    // Output push-pull
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) &= ~(11u << 6u);  // clear speed
    memory(GPIO_A_BASE + GPIO_X_OSPEEDR) |= (11u << 6u);   // very HIGH Speed
}

void init_lpuart() {
    hal::address<hal::stm::stm32g4::mcu_info::APBENR, 0>()
        ->apb12.add<hal::stm::stm32g4::mcu_info::APBENR::APB1ENR2::LPUART1EN>();
    // memory(LPUART_BASE + LPUART_CR1) |= (1u << 29u);            // enable fifo
    memory(LPUART_BASE + LPUART_CR1) &= ~(1u << 28u);           //  1 Start bit, 8 Data bits, n Stop bit
    memory(LPUART_BASE + LPUART_CR1) &= ~(1u << 12u);           //  1 Start bit, 8 Data bits, n Stop bit
    memory(LPUART_BASE + LPUART_CR1) &= ~(1u << 10u);           //  no parity
    memory(LPUART_BASE + LPUART_CR2) &= ~(0b11u << 12u);        // 1 stop bit
    memory(LPUART_BASE + LPUART_BRR) = 16'000'000u / (115200);  // 115200 baud
    memory(LPUART_BASE + LPUART_CR1) |= (1u << 0u);             // enable uart
    memory(LPUART_BASE + LPUART_CR1) |= (1u << 3u);             // enable tx
    // memory(LPUART_BASE + LPUART_CR1) |= (1u << 2u);             // enable rx
    // memory(RCC_BASE + LPUART1SEL) = static_cast<uint32_t>(UARTCLK::PCLK);
}

/****************/

/***** UART *****/
template<auto txpin, auto rxpin, typename uart>
void init_uart_pin() {
    using port_a = hal::periphery::gpio<mcu_ns::A, used_mcu>;

    // enable GPIO clock
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();

    // alternative function mode
    constexpr auto af_tx = used_mcu::find_af<mcu_ns::A, txpin, uart, used_mcu::uart::uart_pin_types::TX>();
    constexpr auto af_rx = used_mcu::find_af<mcu_ns::A, rxpin, uart, used_mcu::uart::uart_pin_types::RX>();
    port_a::set_alternative_function<af_tx, txpin>();
    port_a::set_alternative_function<af_rx, rxpin>();

    // Set GPIO (txpin/rxpin) speed +  push/pull
    port_a::set_speed<gpio_values::speed::VERY_HIGH_SPEED, txpin, rxpin>();
    port_a::set_type<gpio_values::type::PUSH_PULL, txpin, rxpin>();
}

template<auto base_addr>
void init_uart() {
    if constexpr (base_addr == UART_BASE) {
        hal::address<hal::stm::stm32g4::mcu_info::APBENR, 0>()
            ->apb2.add<hal::stm::stm32g4::mcu_info::APBENR::APB2ENR::USART1EN>();
    } else {
        hal::address<hal::stm::stm32g4::mcu_info::APBENR, 0>()
            ->apb11.add<hal::stm::stm32g4::mcu_info::APBENR::APB1ENR1::USART2EN>();
    }
    memory(base_addr + UART_CR1) &= ~(1u << 28u);           //  1 Start bit, 8 Data bits, n Stop bit
    memory(base_addr + UART_CR1) &= ~(1u << 12u);           //  1 Start bit, 8 Data bits, n Stop bit
    memory(base_addr + UART_BRR) = 16'000'000u / (115200);  // 115200 baud
    memory(base_addr + UART_CR2) &= ~(0b11u << 12u);        // 9600 baud
    memory(base_addr + UART_CR3) = 0;
    memory(base_addr + UART_CR1) |= (1u << 0u);  // enable uart
    memory(base_addr + UART_CR1) |= (1u << 3u);  // enable tx
    // memory(UART_BASE + UART_CR1) |= (1u << 2u); // enable tx
}
/****************/

/**
 * @brief  The application entry point.
 * @retval int
 */
int main() {
    using namespace hal::cordic;
    using port_a = hal::periphery::gpio<mcu_ns::A, used_mcu>;
    using uart_two = hal::periphery::uart<mcu_ns::uart_nr::two, used_mcu>;
    using cordic_one = hal::cordic::cordic<mcu_ns::cordic_nr::one, mcu_ns::mcu_info>;
    // SystemClock_Config();
    // memory(RCC_BASE + RCC_AHB2ENR) |= 1u;
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb1.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB1ENR::CORDIC>();
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();

    /*init_led();
    init_timer();
    init_pwm();*/

    // init_uart_pin();
    // init_uart();
    // init_lpuart_pin();
    // init_lpuart();

    init_uart_pin<9u, 10u, mcu_ns::uart_nr::one>();
    init_uart<UART_BASE>();

    init_uart_pin<2u, 3u, mcu_ns::uart_nr::two>();
    init_uart<UART2_BASE>();

    using cc = cordic_config<precision::q1_31>;
    operation<cc, operation_type::single, functions::cosine> op;
    operation<cc, operation_type::single, functions::sine> op2;
    operation<cc, operation_type::single, functions::modulus> op3;

    // memory(GPIO_A_BASE + GPIO_X_MODER) &= ~(0b11u << (10 * 2u));
    // memory(GPIO_A_BASE + GPIO_X_MODER) |= (0b1u << (10 * 2u));

    int deg = 0;

    port_a::set_port_mode<gpio_values::modes::OUTPUT, 5>();

    while (true) {
        // memory(LPUART_BASE + LPUART_TDR) = 'U';
        port_a::pin<5>::on();
        // memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << 5u);
        delay_ms(250);
        // memory(GPIO_A_BASE + GPIO_X_BSRR) = (1u << (5u + 16));
        port_a::pin<5>::off();
        delay_ms(250);
        // while((memory(LPUART_BASE + LPUART_ISR) & (1u << 6u)) >> 6u != 1);
        int16_t rdeg = deg - 180;
        op.arg1(angle<precision::q1_31>{degrees{rdeg}});
        op2.arg1(angle<precision::q1_31>{degrees{rdeg}});

        auto float_val = static_cast<float>(cordic_one::calculate(op).result());
        auto float_val2 = static_cast<float>(cordic_one::calculate(op2).result());
        vec2f<precision::q1_31> v{x_coord{float_val}, y_coord{float_val2}};
        op3.arg(v);

        auto float_val3 = static_cast<double>(cordic_one::calculate(op3).secondary_result()) *
                          M_PI;  // One has to multiply with M_PI to get the result in radians
        uart_two::printf<256>(
            "cos(%d) * 1000 = %d sin(%d) * 1000 = %d atan2(%d * 1000, %d * 1000) = %d * 1000 == %d * 1000 ? "
            "\r\n",
            rdeg, static_cast<int>(float_val * 1000), rdeg, static_cast<int>(float_val2 * 1000),
            static_cast<int>((float)v.y() * 1000), static_cast<int>((float)v.x() * 1000),
            static_cast<int>(float_val3 * 1000), static_cast<int>(std::atan2(float_val2, float_val) * 1000));
        deg = (deg + 1) % 360;
        delay_ms(50);
        // memory(LPUART_BASE + 0x20) |= (1u << 2u);
    }
}

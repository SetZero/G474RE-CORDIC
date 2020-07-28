//
// Created by Sebastian on 28.07.2020.
//

#include "playground.h"

/***** TIMER *****/
[[nodiscard]] inline uint32_t &memory(const uint32_t loc) { return *reinterpret_cast<uint32_t *>(loc); }
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

/***** LPUART *****/
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
    memory(LPUART_BASE + LPUART_CR1) |= (1u << 29u);            // enable fifo
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

/***** UART *****/
template<auto txpin, auto rxpin, typename uart>
void init_uart_pin() {
    using port_a = hal::periphery::gpio<hal::stm::stm32g4::A, used_mcu>;

    // enable GPIO clock
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();

    // alternative function mode
    constexpr auto af_tx = hal::stm::stm32g4::g474re::find_af<hal::stm::stm32g4::A, txpin, uart,
                                                              hal::stm::stm32g4::g474re::uart::uart_pin_types::TX>();
    constexpr auto af_rx = hal::stm::stm32g4::g474re::find_af<hal::stm::stm32g4::A, rxpin, uart,
                                                              hal::stm::stm32g4::g474re::uart::uart_pin_types::RX>();
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
    memory(base_addr + UART_CR1) &= ~(1u << 12u);           //  1 Start bit, 8 Data bits, n Stop bit*/
    memory(base_addr + UART_CR2) &= ~(0b11u << 12u);        // 1 stop bit
    memory(base_addr + UART_BRR) = 16'000'000u / (115200);  // 115200 baud
    memory(base_addr + UART_CR3) = 0;
    memory(base_addr + UART_CR1) |= (1u << 0u);  // enable uart
    memory(base_addr + UART_CR1) |= (1u << 3u);  // enable tx
    memory(UART_BASE + UART_CR1) |= (1u << 2u);  // enable tx
} /* REGISTER BASE ADDRESSES */

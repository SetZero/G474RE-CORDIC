#pragma once

#include <hal/stm32/stm32g4.h>
#include <concepts>

#include "hal_info.h"

namespace hal::periphery {

    // TODO: add real uart concept
    template<typename MCU, typename PIN>
    concept uart_mcu = specialized_mcu<MCU, PIN>;

    namespace detail {
        struct uart_component {};

        template<mcu_with_vendor_info MCU>
        struct stm_mcu_mapper<MCU, uart_component> {
            template<typename UartNr>
            struct mapper {};
        };
    }  // namespace detail

    template<typename UartNr, uart_mcu<UartNr> UsedMCU>
    requires(hal::info::vendor_information<typename UsedMCU::base_mcu>::vendor == info::vendors::STM) class uart {
       private:
        using MCU = UsedMCU::base_mcu;
        static inline constexpr auto uart_registers = hal::address<typename MCU::UART, UartNr>;
        using uart_detail = typename detail::stm_mcu_mapper<MCU, detail::uart_component>::mapper<UartNr>;
        using uart_type = MCU::UART;

       public:
        uart() = delete;

        template<gpio_pin TXPin, gpio_pin RXPin, auto Baudrate, auto DataBits = 8, auto StopBits = 0>
        requires(DataBits >= 7 && DataBits <= 9 && StopBits >= 0 && StopBits <= 2)
        static void init() {
            uart_registers()->cr1.template set_value<MCU::UART::CR::UE>(false);
            //TODO: Fixme
            hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()->ahb2.add<MCU::AHBENR::AHB2ENR::GPIOA>();

            TXPin::template set_alternative_function<UartNr, UsedMCU::uart::uart_pin_types::TX>();
            RXPin::template set_alternative_function<UartNr, UsedMCU::uart::uart_pin_types::RX>();

            TXPin::template set_speed<gpio_values::speed::VERY_HIGH_SPEED>();
            RXPin::template set_speed<gpio_values::speed::VERY_HIGH_SPEED>();

            TXPin::template set_type<gpio_values::type::PUSH_PULL>();
            RXPin::template set_type<gpio_values::type::PUSH_PULL>();

            // TODO: Changme
            hal::address<hal::stm::stm32g4::mcu_info::APBENR, 0>()
                ->apb11.add<hal::stm::stm32g4::mcu_info::APBENR::APB1ENR1::USART2EN>();

            if constexpr(DataBits == 7) {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(false);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(false);
            } else if(DataBits == 8) {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(true);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(false);
            } else {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(false);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(true);
            }
        }

        static void printc(char value) { uart_registers()->tdr.template set_value<0>(value); }

        template<auto BufSize, typename... Args>
        static void printf(const char *format, Args &&... args) {
            char buf[BufSize];

            const auto print_num = snprintf(buf, BufSize, format, args...);

            for (auto i = 0; i < print_num; ++i) {
                while (!uart_registers()->isr.template get_value<uart_type::ISR::TXFNF>())
                    ;
                uart_registers()->tdr.template set_value<0>(buf[i]);
            }
        }
    };
}  // namespace hal::periphery

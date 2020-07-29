#pragma once

#include <hal/stm32/stm32g4.h>

#include <concepts>

#include "hal_info.h"

namespace hal::periphery {

    // TODO: add real uart concept
    template<typename MCU, typename PIN>
    concept uart_mcu = specialized_mcu<MCU, PIN>;

    namespace uart_values {
        enum class stop { STOP_1, STOP_0_5, STOP_2, STOP_1_5 };
    }

    namespace detail {
        struct uart_component {};

        template<mcu_with_vendor_info MCU>
        struct stm_mcu_mapper<MCU, uart_component> {
            using mcu = MCU;

            template<typename UartNr>
            struct mapper {
                using stop_mapper_pair = std::pair<uart_values::stop, typename mcu::UART::stop>;
                static inline constexpr value_mapper stop_mapper{
                    stop_mapper_pair{uart_values::stop::STOP_1, mcu::UART::stop::STOP_1},
                    stop_mapper_pair{uart_values::stop::STOP_0_5, mcu::UART::stop::STOP_0_5},
                    stop_mapper_pair{uart_values::stop::STOP_1_5, mcu::UART::stop::STOP_1_5},
                    stop_mapper_pair{uart_values::stop::STOP_2, mcu::UART::stop::STOP_2}};
            };
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

        template<gpio_pin TXPin, gpio_pin RXPin, auto Baudrate = 9600, auto DataBits = 8, uart_values::stop StopBits = uart_values::stop::STOP_1>
        requires(DataBits >= 7 && DataBits <= 9) static void init() {
            uart_registers()->cr1.template set_value<MCU::UART::CR::UE>(false);
            // TODO: Fixme
            mcu_features<MCU>::template enable_clock<features::hal_features::GPIOA>();

            TXPin::template set_alternative_function<UartNr, UsedMCU::uart::uart_pin_types::TX>();
            RXPin::template set_alternative_function<UartNr, UsedMCU::uart::uart_pin_types::RX>();

            TXPin::template set_speed<gpio_values::speed::VERY_HIGH_SPEED>();
            RXPin::template set_speed<gpio_values::speed::VERY_HIGH_SPEED>();

            TXPin::template set_type<gpio_values::type::PUSH_PULL>();
            RXPin::template set_type<gpio_values::type::PUSH_PULL>();

            mcu_features<MCU>::template enable_clock<features::hal_features::UART, UartNr>();

            if constexpr (DataBits == 8) {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(false);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(false);
            } else if (DataBits == 7) {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(true);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(false);
            } else if (DataBits == 9) {
                uart_registers()->cr1.template set_value<MCU::UART::CR::M1>(false);
                uart_registers()->cr1.template set_value<MCU::UART::CR::M0>(true);
            }

            uart_registers()->cr1.template set_value<MCU::UART::CR::PCE>(false);
            uart_registers()->cr2.template set_value<MCU::UART::CR2::STOP>(uart_detail::stop_mapper[StopBits]);
            uart_registers()->brr.template set_value<MCU::UART::BRR::BRR>(UsedMCU::frequency / Baudrate);

            uart_registers()->cr1.template set_value<MCU::UART::CR::UE>(true);
            uart_registers()->cr1.template set_value<MCU::UART::CR::TE>(true);
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

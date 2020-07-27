#pragma once

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

        template<typename RXPin, typename TXPin, auto Baudrate>
        static void init() {

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

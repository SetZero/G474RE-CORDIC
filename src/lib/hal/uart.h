#pragma once

#include "hal_info.h"

namespace hal::periphery {

    // TODO: add real uart concept
    template<typename MCU, typename PIN>
    concept uart_mcu = stm_mcu<MCU, PIN>;

    namespace detail {
        struct uart_component {};

        template<mcu_with_vendor_info MCU>
        struct stm_mcu_mapper<MCU, uart_component> {
            template<typename UartNr>
            struct mapper {};
        };
    }  // namespace detail

    template<typename UartNr, uart_mcu<UartNr> MCU>
    requires(MCU::vendor_information::vendors == info::vendors::STM) class uart {
       private:
        using uart_detail = typename detail::stm_mcu_mapper<MCU, detail::uart_component>::mapper<UartNr>;

       public:
        uart() = delete;
    };
}  // namespace hal::periphery

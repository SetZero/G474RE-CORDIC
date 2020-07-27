//
// Created on 26.07.2020.
//

#pragma once

#include <hal/gpio.h>

#include "../../../type_mapper.h"

namespace hal::stm::stm32g4 {

    namespace detail {
        using base_mcu = mcu_info;
        using gpio_base = hal::periphery::gpio_values::alternative_function;
        using uart_base = base_mcu::UART;

        template<typename T, uint32_t number, typename function_number, auto function>
        struct find_af;

        // TODO: ADD Port D, E, F, G

        template<typename T, uint32_t number, typename function_number, auto function>
        struct af_type;

        type_mapper mapper{
            type_value_pair<find_af<A, 0, uart_nr::two, uart_base::uart_pin_types::CTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 0, uart_nr::two, uart_base::uart_pin_types::CTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 1, uart_nr::two, uart_base::uart_pin_types::RTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 2, uart_nr::two, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 3, uart_nr::two, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 4, uart_nr::two, uart_base::uart_pin_types::CK>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 8, uart_nr::one, uart_base::uart_pin_types::CK>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 9, uart_nr::one, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 10, uart_nr::one, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 11, uart_nr::one, uart_base::uart_pin_types::CTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 12, uart_nr::one, uart_base::uart_pin_types::RTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 13, uart_nr::three, uart_base::uart_pin_types::CTS>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 14, uart_nr::two, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<A, 15, uart_nr::two, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 3, uart_nr::two, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 4, uart_nr::two, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 5, uart_nr::two, uart_base::uart_pin_types::CK>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 6, uart_nr::one, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 7, uart_nr::one, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 8, uart_nr::three, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 9, uart_nr::three, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 10, uart_nr::three, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 11, uart_nr::three, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 12, uart_nr::three, uart_base::uart_pin_types::CK>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 13, uart_nr::three, uart_base::uart_pin_types::CTS>, gpio_base::AF7>{},
            type_value_pair<af_type<B, 14, uart_nr::three, uart_base::uart_pin_types::RTS>, gpio_base::AF7>{},
            type_value_pair<af_type<C, 4, uart_nr::one, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<C, 5, uart_nr::one, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<C, 10, uart_nr::three, uart_base::uart_pin_types::TX>, gpio_base::AF7>{},
            type_value_pair<af_type<C, 11, uart_nr::three, uart_base::uart_pin_types::RX>, gpio_base::AF7>{},
            type_value_pair<af_type<C, 12, uart_nr::three, uart_base::uart_pin_types::CK>, gpio_base::AF7>{}};
    }  // namespace detail

    class g474re {
        using base_mcu = mcu_info;
        using gpio_base = hal::periphery::gpio_values::alternative_function;
        using uart_base = base_mcu::UART;

       public:
        template<typename T, uint32_t number, typename function_number, auto function>
        requires(number < 31) static constexpr gpio_base find_af() {
            return detail::mapper.lookup_type<detail::af_type<T, number, function_number, function>>();
        }
    };
}  // namespace hal::stm::stm32g4

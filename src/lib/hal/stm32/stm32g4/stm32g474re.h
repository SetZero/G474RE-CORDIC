//
// Created on 26.07.2020.
//

#pragma once

#include <hal/gpio.h>

#include "../../../type_mapper.h"

namespace hal::stm::stm32g4 {

    namespace detail {
        using base_mcu = mcu_info;
        using af_name = hal::periphery::gpio_values::alternative_function;
        using uart = base_mcu::UART;
        using cordic = base_mcu::CORDIC;

        // TODO: ADD Port D, E, F, G

        template<typename T, uint32_t number, typename function_number, auto function>
        struct af_type;

        type_mapper mapper{
            type_value_pair<af_type<A, 0, uart_nr::two, uart::uart_pin_types::CTS>, af_name::AF7>{},
            type_value_pair<af_type<A, 1, uart_nr::two, uart::uart_pin_types::RTS>, af_name::AF7>{},
            type_value_pair<af_type<A, 2, uart_nr::two, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<A, 3, uart_nr::two, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<A, 4, uart_nr::two, uart::uart_pin_types::CK>, af_name::AF7>{},
            type_value_pair<af_type<A, 8, uart_nr::one, uart::uart_pin_types::CK>, af_name::AF7>{},
            type_value_pair<af_type<A, 9, uart_nr::one, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<A, 10, uart_nr::one, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<A, 11, uart_nr::one, uart::uart_pin_types::CTS>, af_name::AF7>{},
            type_value_pair<af_type<A, 12, uart_nr::one, uart::uart_pin_types::RTS>, af_name::AF7>{},
            type_value_pair<af_type<A, 13, uart_nr::three, uart::uart_pin_types::CTS>, af_name::AF7>{},
            type_value_pair<af_type<A, 14, uart_nr::two, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<A, 15, uart_nr::two, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<B, 3, uart_nr::two, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<B, 4, uart_nr::two, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<B, 5, uart_nr::two, uart::uart_pin_types::CK>, af_name::AF7>{},
            type_value_pair<af_type<B, 6, uart_nr::one, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<B, 7, uart_nr::one, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<B, 8, uart_nr::three, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<B, 9, uart_nr::three, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<B, 10, uart_nr::three, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<B, 11, uart_nr::three, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<B, 12, uart_nr::three, uart::uart_pin_types::CK>, af_name::AF7>{},
            type_value_pair<af_type<B, 13, uart_nr::three, uart::uart_pin_types::CTS>, af_name::AF7>{},
            type_value_pair<af_type<B, 14, uart_nr::three, uart::uart_pin_types::RTS>, af_name::AF7>{},
            type_value_pair<af_type<C, 4, uart_nr::one, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<C, 5, uart_nr::one, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<C, 10, uart_nr::three, uart::uart_pin_types::TX>, af_name::AF7>{},
            type_value_pair<af_type<C, 11, uart_nr::three, uart::uart_pin_types::RX>, af_name::AF7>{},
            type_value_pair<af_type<C, 12, uart_nr::three, uart::uart_pin_types::CK>, af_name::AF7>{}};
    }  // namespace detail

    class g474re {
       private:
        using af_type = hal::periphery::gpio_values::alternative_function;

       public:
        using base_mcu = mcu_info;
        using uart = base_mcu::UART;
        using cordic = base_mcu::CORDIC;
        using gpio = base_mcu::GPIO;

        template<typename Port, uint32_t pin, typename function_number, auto function_type>
        requires(pin <= 31) static constexpr af_type find_af() {
            return detail::mapper.lookup_type<detail::af_type<Port, pin, function_number, function_type>>();
        }
    };
}  // namespace hal::stm::stm32g4

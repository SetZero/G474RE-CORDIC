//
// Created by Sebastian on 26.07.2020.
//

#pragma once


#include <hal/gpio.h>

namespace hal::stm::stm32g4 {

    namespace detail {
        using base_mcu = mcu_info;
        using gpio_base = hal::periphery::gpio_values::alternative_function;
        using uart_base = base_mcu::UART;

        template<typename T, uint32_t number, typename function_number, auto function>
        struct find_af;

        template<>
        struct find_af<A, 0, uart_nr::two, uart_base::uart_pin_types::CTS> {
            static constexpr auto value = hal::periphery::gpio_values::alternative_function::AF7;
        };

        template<>
        struct find_af<A, 1, uart_nr::two, uart_base::uart_pin_types::RTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 2, uart_nr::two, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 3, uart_nr::two, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 4, uart_nr::two, uart_base::uart_pin_types::CK> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 8, uart_nr::one, uart_base::uart_pin_types::CK> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 9, uart_nr::one, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 10, uart_nr::one, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 11, uart_nr::one, uart_base::uart_pin_types::CTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 12, uart_nr::one, uart_base::uart_pin_types::RTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 13, uart_nr::three, uart_base::uart_pin_types::CTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 14, uart_nr::two, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<A, 15, uart_nr::two, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 3, uart_nr::two, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 4, uart_nr::two, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 5, uart_nr::two, uart_base::uart_pin_types::CK> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 6, uart_nr::one, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 7, uart_nr::one, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 8, uart_nr::three, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 9, uart_nr::three, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 10, uart_nr::three, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 11, uart_nr::three, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 12, uart_nr::three, uart_base::uart_pin_types::CK> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 13, uart_nr::three, uart_base::uart_pin_types::CTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<B, 14, uart_nr::three, uart_base::uart_pin_types::RTS> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<C, 4, uart_nr::one, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<C, 5, uart_nr::one, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<C, 10, uart_nr::three, uart_base::uart_pin_types::TX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<C, 11, uart_nr::three, uart_base::uart_pin_types::RX> {
            static constexpr auto value = gpio_base::AF7;
        };

        template<>
        struct find_af<C, 12, uart_nr::three, uart_base::uart_pin_types::CK> {
            static constexpr auto value = gpio_base::AF7;
        };
        // TODO: ADD Port D, E, F, G
    }

    class g474re {
        using base_mcu = mcu_info;
        using gpio_base = hal::periphery::gpio_values::alternative_function;
        using uart_base = base_mcu::UART;
    public:
        template<typename T, uint32_t number, typename function_number, auto function>
        requires (number < 31) static constexpr gpio_base find_af() {
            return detail::find_af<T, number, function_number, function>::value;
        }
    };
}



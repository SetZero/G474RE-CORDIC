//
// Created by Sebastian on 14.07.2020.
//

#pragma once
#include "../utils.h"
#include "hal/hal_info.h"
#include "stm32/stm32g4.h"

template<typename MCU, typename PIN>
concept gpio_mcu = stm_mcu<MCU, PIN>;

// TODO: Variadic Template Version
namespace hal::periphery {
    namespace gpio_values {
        enum class modes { OUTPUT, INPUT, ALTERNATIVE_FUNCTION, ANALOG };  // TODO: Remove direct access to AF
        enum class type { PUSH_PULL = 0, OPEN_DRAIN = 1 };
        enum class speed { LOW_SPEED = 0, MEDIUM_SPEED = 1, HIGH_SPEED = 2, VERY_HIGH_SPEED = 3 };
        enum class alternative_function {
            AF0 = 0,
            AF1,
            AF2,
            AF3,
            AF4,
            AF5,
            AF6,
            AF7,
            AF8,
            AF9,
            AF10,
            AF11,
            AF12,
            AF13,
            AF14,
            AF15
        };
    }  // namespace gpio_values

    namespace detail {
        struct gpio_component {};

        template<mcu_with_vendor_info MCU>
        struct stm_mcu_mapper<MCU, gpio_component> {
            using mcu = MCU;
            template<typename gpio_port>
            struct mapper {
                using mode_mapper_pair = std::pair<gpio_values::modes, typename mcu::GPIO::MODER>;
                static inline constexpr value_mapper mode_mapper{
                    mode_mapper_pair{gpio_values::modes::OUTPUT, mcu::GPIO::MODER::GP_OUT},
                    mode_mapper_pair{gpio_values::modes::INPUT, mcu::GPIO::MODER::INPUT},
                    mode_mapper_pair{gpio_values::modes::ALTERNATIVE_FUNCTION, mcu::GPIO::MODER::ALTERNATIVE_FUNCTION},
                    mode_mapper_pair{gpio_values::modes::ANALOG, mcu::GPIO::MODER::ANALOG}};
                using type_mapper_pair = std::pair<gpio_values::type, typename mcu::GPIO::OTYPER>;
                static inline constexpr value_mapper type_mapper{
                    type_mapper_pair{gpio_values::type::PUSH_PULL, mcu::GPIO::OTYPER::PUSH_PULL},
                    type_mapper_pair{gpio_values::type::OPEN_DRAIN, mcu::GPIO::OTYPER::OPEN_DRAIN}};
                using speed_mapper_pair = std::pair<gpio_values::speed, typename mcu::GPIO::OSPEEDR>;
                static inline constexpr value_mapper speed_mapper{
                    speed_mapper_pair{gpio_values::speed::LOW_SPEED, mcu::GPIO::OSPEEDR::LOW_SPEED},
                    speed_mapper_pair{gpio_values::speed::MEDIUM_SPEED, mcu::GPIO::OSPEEDR::MEDIUM_SPEED},
                    speed_mapper_pair{gpio_values::speed::HIGH_SPEED, mcu::GPIO::OSPEEDR::HIGH_SPEED},
                    speed_mapper_pair{gpio_values::speed::VERY_HIGH_SPEED, mcu::GPIO::OSPEEDR::VERY_HIGH_SPEED}};
                using af_mapper_pair = std::pair<gpio_values::alternative_function, typename mcu::GPIO::AFR>;
                static inline constexpr value_mapper af_mapper{
                    af_mapper_pair{gpio_values::alternative_function::AF0, mcu::GPIO::AFR::AF0},
                    af_mapper_pair{gpio_values::alternative_function::AF1, mcu::GPIO::AFR::AF1},
                    af_mapper_pair{gpio_values::alternative_function::AF2, mcu::GPIO::AFR::AF2},
                    af_mapper_pair{gpio_values::alternative_function::AF3, mcu::GPIO::AFR::AF3},
                    af_mapper_pair{gpio_values::alternative_function::AF4, mcu::GPIO::AFR::AF4},
                    af_mapper_pair{gpio_values::alternative_function::AF5, mcu::GPIO::AFR::AF5},
                    af_mapper_pair{gpio_values::alternative_function::AF6, mcu::GPIO::AFR::AF6},
                    af_mapper_pair{gpio_values::alternative_function::AF7, mcu::GPIO::AFR::AF7},
                    af_mapper_pair{gpio_values::alternative_function::AF8, mcu::GPIO::AFR::AF8},
                    af_mapper_pair{gpio_values::alternative_function::AF9, mcu::GPIO::AFR::AF9},
                    af_mapper_pair{gpio_values::alternative_function::AF10, mcu::GPIO::AFR::AF10},
                    af_mapper_pair{gpio_values::alternative_function::AF11, mcu::GPIO::AFR::AF11},
                    af_mapper_pair{gpio_values::alternative_function::AF12, mcu::GPIO::AFR::AF12},
                    af_mapper_pair{gpio_values::alternative_function::AF13, mcu::GPIO::AFR::AF13},
                    af_mapper_pair{gpio_values::alternative_function::AF14, mcu::GPIO::AFR::AF14},
                    af_mapper_pair{gpio_values::alternative_function::AF15, mcu::GPIO::AFR::AF15}};
            };
        };

    }  // namespace detail

    template<typename gpio_port, specialized_mcu<gpio_port> used_mcu>
    requires(hal::info::vendor_information<typename used_mcu::base_mcu>::vendor == info::vendors::STM) class gpio {
       private:
        using mcu = used_mcu::base_mcu;
        static inline constexpr auto port = hal::address<typename mcu::GPIO, gpio_port>;
        using mcu_detail = typename detail::stm_mcu_mapper<mcu, detail::gpio_component>::mapper<gpio_port>;

       public:
        gpio() = delete;

        template<gpio_values::modes gpio_mode, typename decltype(port()->moder)::value_type... pins>
        static void set_port_mode() {
            port()->moder.template clear_add<mcu_detail::mode_mapper[gpio_mode], pins...>();
        }

        template<typename decltype(port()->bssr_set_io)::value_type... io>
        static void on() {
            port()->bssr_set_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        template<typename decltype(port()->bssr_clear_io)::value_type... io>
        static void off() {
            port()->bssr_clear_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        static auto get() { return *port()->idr; }

        template<gpio_values::type gpio_type, typename decltype(port()->otyper)::value_type... io>
        static void set_type() {
            port()->otyper.template add<mcu_detail::type_mapper[gpio_type], io...>();
        }

        template<gpio_values::speed gpio_speed, typename decltype(port()->ospeedr)::value_type... io>
        static void set_speed() {
            port()->ospeedr.template add<mcu_detail::speed_mapper[gpio_speed], io...>();
        }

        // TODO: ADD AF LUT
        template<gpio_values::alternative_function af, typename decltype(port()->afr)::value_type... io>
        static void set_alternative_function() {
            set_port_mode<gpio_values::modes::ALTERNATIVE_FUNCTION, io...>();
            port()->afr.template add<mcu_detail::af_mapper[af], io...>();
        }

        template<typename function_number, auto function, typename decltype(port()->afr)::value_type io>
        static void set_alternative_function() {
            set_port_mode<gpio_values::modes::ALTERNATIVE_FUNCTION, io>();
            port()->afr.template add<used_mcu::template find_af<gpio_port, io, function_number, function>, io>();
        }
    };
}  // namespace hal::periphery

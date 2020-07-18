//
// Created by Sebastian on 14.07.2020.
//

#pragma once
#include "../utils.h"
#include "stm32/stm32g4.h"

// TODO: remove this
#define __SELECTED_MCU__ hal::stm::stm32g4::peripherals

// TODO: Variadic Template Version
namespace hal::periphery {
    namespace gpio_values {
        enum class modes { OUTPUT, INPUT, ALTERNATIVE_FUNCTION, ANALOG };  // TODO: Remove direct access to AF
        enum class type { PUSH_PULL = 0, OPEN_DRAIN = 1 };
        enum class speed { LOW_SPEED = 0, MEDIUM_SPEED = 1, HIGH_SPEED = 2, VERY_HIGH_SPEED = 3 };
    }

    namespace detail {
        template<typename gpio_port, mcu_with_vendor_info mcu>
            requires (mcu::vendor_information::vendors == info::vendors::STM)
        struct stm_mcu_mapper {
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
        };
    }

    template<typename gpio_port, gpio_mcu<gpio_port> mcu = __SELECTED_MCU__>
        requires (mcu::vendor_information::vendors == info::vendors::STM)
    class gpio {
       private:
        static inline constexpr auto port = hal::address<typename mcu::GPIO, gpio_port>;
        using mcu_detail = detail::stm_mcu_mapper<gpio_port, mcu>;

       public:
        gpio() = delete;

        template<gpio_values::modes gpio_mode, typename decltype(port()->moder)::value_type... pins>
        static void set_port_mode() {
            port()->moder.template clear_add<mcu_detail::mode_mapper[gpio_mode], pins...>();
        }

        template<typename decltype(port()->bssr_set_io)::value_type... io>
        static void on() {
            return port()->bssr_set_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        template<typename decltype(port()->bssr_clear_io)::value_type... io>
        static void off() {
            return port()->bssr_clear_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        template<gpio_values::type gpio_type, typename decltype(port()->otyper)::value_type... io>
        static void set_type() {
            return port()->otyper.template add<mcu_detail::type_mapper[gpio_type], io...>();
        }

        template<gpio_values::speed gpio_speed, typename decltype(port()->ospeedr)::value_type... io>
        static void set_speed() {
            return port()->ospeedr.template add<mcu_detail::speed_mapper[gpio_speed], io...>();
        }
    };
}  // namespace hal::periphery

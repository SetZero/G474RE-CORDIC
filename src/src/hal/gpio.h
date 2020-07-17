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
    template<typename gpio_port, gpio_mcu<gpio_port> mcu = __SELECTED_MCU__>
    class gpio {
       public:
        enum class modes { OUTPUT, INPUT, ALTERNATIVE_FUNCTION, ANALOG };  // TODO: Remove direct access to AF
        enum class type { PUSH_PULL = 0, OPEN_DRAIN = 1 };
        enum class speed { LOW_SPEED = 0, MEDIUM_SPEED = 1, HIGH_SPEED = 2, VERY_HIGH_SPEED = 3 };

       private:
        static inline constexpr auto port = hal::address<typename mcu::GPIO, gpio_port>;
        using mode_mapper_pair = std::pair<modes, typename mcu::GPIO::MODER>;
        static inline constexpr value_mapper mode_mapper{
            mode_mapper_pair{modes::OUTPUT, mcu::GPIO::MODER::GP_OUT},
            mode_mapper_pair{modes::INPUT, mcu::GPIO::MODER::INPUT},
            mode_mapper_pair{modes::ALTERNATIVE_FUNCTION, mcu::GPIO::MODER::ALTERNATIVE_FUNCTION},
            mode_mapper_pair{modes::ANALOG, mcu::GPIO::MODER::ANALOG}};
        using type_mapper_pair = std::pair<type, typename mcu::GPIO::OTYPER>;
        static inline constexpr value_mapper type_mapper{
            type_mapper_pair{type::PUSH_PULL, mcu::GPIO::OTYPER::PUSH_PULL},
            type_mapper_pair{type::OPEN_DRAIN, mcu::GPIO::OTYPER::OPEN_DRAIN}};
        using speed_mapper_pair = std::pair<speed, typename mcu::GPIO::OSPEEDR>;
        static inline constexpr value_mapper speed_mapper{
            speed_mapper_pair{speed::LOW_SPEED, mcu::GPIO::OSPEEDR::LOW_SPEED},
            speed_mapper_pair{speed::MEDIUM_SPEED, mcu::GPIO::OSPEEDR::MEDIUM_SPEED},
            speed_mapper_pair{speed::HIGH_SPEED, mcu::GPIO::OSPEEDR::HIGH_SPEED},
            speed_mapper_pair{speed::VERY_HIGH_SPEED, mcu::GPIO::OSPEEDR::VERY_HIGH_SPEED}};

       public:
        gpio() = delete;

        template<modes gpio_mode, typename decltype(port()->moder)::value_type... pins>
        static void set_port_mode() {
            port()->moder.template clear_add<mode_mapper[gpio_mode], pins...>();
        }

        template<typename decltype(port()->bssr_set_io)::value_type... io>
        static void on() {
            return port()->bssr_set_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        template<typename decltype(port()->bssr_clear_io)::value_type... io>
        static void off() {
            return port()->bssr_clear_io.template add<mcu::GPIO::BSSR::SET, io...>();
        }

        template<type gpio_type, typename decltype(port()->otyper)::value_type... io>
        static void set_type() {
            return port()->otyper.template add<type_mapper[gpio_type], io...>();
        }

        template<speed gpio_speed, typename decltype(port()->ospeedr)::value_type... io>
        static void set_speed() {
            return port()->ospeedr.template add<speed_mapper[gpio_speed], io...>();
        }
    };
}  // namespace hal::periphery

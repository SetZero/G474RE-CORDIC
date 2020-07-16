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
        enum class modes { OUTPUT, INPUT, ALTERNATIVE_FUNCTION, ANALOG }; // TODO: Remove direct access to AF

       private:
        using func_mapper_pair = std::pair<modes, typename mcu::GPIO::MODER>;
        static inline constexpr auto port = hal::address<typename mcu::GPIO, gpio_port>;

       public:
        gpio() = delete;
        static inline constexpr value_mapper func_mapper{func_mapper_pair{modes::OUTPUT, mcu::GPIO::MODER::GP_OUT}};

        template<typename decltype(port()->moder)::value_type io, modes gpio_mode>
        static void set_port_mode() {
            return port()->moder.template clear_add<io, func_mapper[gpio_mode]>();
        }

        template<typename decltype(port()->bssr_set_io)::value_type io>
        static void on() {
            return port()->bssr_set_io.template add<io, mcu::GPIO::BSSR::SET>();
        }

        template<typename decltype(port()->bssr_clear_io)::value_type io>
        static void off() {
            return port()->bssr_clear_io.template add<io, mcu::GPIO::BSSR::SET>();
        }
    };
}  // namespace hal::periphery

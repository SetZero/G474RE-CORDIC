//
// Created by Sebastian on 14.07.2020.
//

#pragma once
#include "../utils.h"
#include "stm32/stm32g4.h"

// TODO: remove this
#define __SELECTED_MCU__ hal::stm::stm32g4::peripherals

namespace hal::periphery {
    template<typename gpio_port, gpio_mcu<gpio_port> mcu = __SELECTED_MCU__>
    class gpio {
        static inline constexpr auto port = hal::address<typename mcu::GPIO, gpio_port>;

       public:
        gpio() = delete;
        static auto& mode() { return port()->moder; }
    };
}  // namespace hal::periphery

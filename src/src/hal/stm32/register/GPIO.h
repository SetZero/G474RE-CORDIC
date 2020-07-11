//
// Created by Sebastian on 10.07.2020.
//

#pragma once

#include <cstdint>

enum class gpio_register_types { MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR, BRR  };

template<typename Component, gpio_register_types reg, typename value_type = uint32_t>
class GPIO {};

template<typename Component>
class GPIO<Component, gpio_register_types::MODER, uint32_t> {
   public:
    enum class MODER { INPUT = 0, GP_OUT = 1, ALTERNATIVE_FUNCTION = 2, ANALOG = 3 };

    GPIO(const GPIO &) = delete;
    GPIO(GPIO &&) = delete;

    GPIO &operator=(const GPIO &) = delete;
    GPIO &operator=(GPIO &&) = delete;

   private:
    volatile uint32_t hw_register;
};

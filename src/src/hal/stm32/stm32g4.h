//
// Created by Sebastian on 30.03.2020.
//

#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/register.h"
#include "hal/stm32/register/cordic.h"

namespace HAL::STM {
    struct A {};
    struct B {};
    struct C {};
    struct D {};
    struct E {};
    struct F {};
    struct G {};

    struct peripherals {
        struct base_address {
            static constexpr inline uintptr_t value = 0x40021000;
        };

        struct GPIO final {
            enum class MODER { INPUT = 0, GP_OUT = 1, ALTERNATIVE_FUNCTION = 2, ANALOG = 3 };

            GPIO() = delete;
            control_register<GPIO, MODER> moder;
            template<typename L>
            struct address;
        };

        // TODO: structure this better somehow
        struct CORDIC {
            using csr_register_type = Cordic::cordic_register<CORDIC, Cordic::register_types::CSR>;
            using wdata_register_type = Cordic::cordic_register<CORDIC, Cordic::register_types::WDATA>;
            using rdata_register_type = Cordic::cordic_register<CORDIC, Cordic::register_types::RDATA>;

            CORDIC() = delete;

            csr_register_type csr;
            wdata_register_type wdata;
            rdata_register_type rdata;

            template<auto N>
            struct address;
        } __attribute__((packed));

        struct AHBENR {
            AHBENR() = delete;

            enum class AHB1ENR : uint32_t {
                DMA1 = (1u << 0u),
                DM2 = (1u << 1u),
                DMAM = (1u << 2u),
                CORDIC = (1u << 3u),
                FMAC = (1u << 4u),
                FLASH = (1u << 8u),
                CRCEN = (1u << 12u),
            };

            enum class AHB2ENR : uint32_t {
                GPIOA = (1u << 0u),
                GPIOB = (1u << 1u),
                GPIOC = (1u << 2u),
                GPIOD = (1u << 3u),
                GPIOE = (1u << 4u),
                GPIOF = (1u << 5u),
                GPIOG = (1u << 6u),
                ADC12 = (1u << 13u),
                ADC345 = (1u << 14u),
                DAC1 = (1u << 16u),
                DAC2 = (1u << 17u),
                DAC3 = (1u << 18u),
                DAC4 = (1u << 19u),
                AES = (1u << 24u),
                RNG = (1u << 26u)
            };

            enum class AHB3ENR : uint32_t {
                FMC = (1u << 0u),
                QSPI = (1u << 8u),
            };

            control_register<AHBENR, AHB1ENR> ahb1;
            control_register<AHBENR, AHB2ENR> ahb2;
            control_register<AHBENR, AHB3ENR> ahb3;

            template<auto N>
            struct address;
        } __attribute__((packed));
    } __attribute__((packed)); /* TODO: Use [[attribute]] Keyword */

    template<>
    struct peripherals::AHBENR::address<0> {
        static constexpr inline uintptr_t value = peripherals::base_address::value + 0x48;
    };

    template<>
    struct peripherals::GPIO::address<A> {
        static constexpr inline uintptr_t value = 0x48000000;
    };

    template<>
    struct peripherals::CORDIC::address<0> {
        static constexpr inline uintptr_t value = 0x40020C00;
    };
}  // namespace HAL::STM

//
// Created by Sebastian on 30.03.2020.
//

#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/hal_info.h"
#include "hal/register.h"
#include "hal/stm32/register/cordic.h"

namespace hal::stm::stm32g4 {

    struct A {};
    struct B {};
    struct C {};
    struct D {};
    struct E {};
    struct F {};
    struct G {};

    namespace uart_nr {
        struct one {};
        struct two {};
        struct three {};
        struct four {};
    }  // namespace uart_nr

    struct mcu_info {
        struct base_address {
            static constexpr inline uintptr_t value = 0x40021000;
        };

        struct GPIO final {
            enum class MODER { INPUT = 0, GP_OUT = 1, ALTERNATIVE_FUNCTION = 2, ANALOG = 3 };
            enum class OTYPER { PUSH_PULL = 0, OPEN_DRAIN = 1 };
            enum class OSPEEDR { LOW_SPEED = 0, MEDIUM_SPEED = 1, HIGH_SPEED = 2, VERY_HIGH_SPEED = 3 };
            enum class PUPDR { NONE = 0, PULLUP = 1, PULLDOWN = 2 };
            enum class BSSR { SET = 1 };
            enum class AFR { AF0 = 0, AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9, AF10, AF11, AF12, AF13, AF14, AF15 };
            enum class BRR { RESET = 1 };

            GPIO() = delete;
            repeated_control_register<GPIO, MODER, uint32_t, 2> moder;
            repeated_control_register<GPIO, OTYPER, uint32_t, 1, 16> otyper;
            repeated_control_register<GPIO, OSPEEDR, uint32_t, 2> ospeedr;
            repeated_control_register<GPIO, PUPDR, uint32_t, 2> pupdr;
            data_register<GPIO, data_register_type::READ_ONLY, uint32_t, uint32_t{0xFFFF}> idr;
            data_register<GPIO, data_register_type::READ_WRITE, uint32_t, uint32_t{0xFFFF}> odr;
            repeated_control_register<GPIO, BSSR, uint16_t, 1> bssr_set_io;
            repeated_control_register<GPIO, BSSR, uint16_t, 1> bssr_clear_io;
            data_register<GPIO, data_register_type::READ_WRITE, uint32_t, uint32_t{0x1FFFF}> lckr;
            repeated_control_register<GPIO, AFR, uint64_t, 4> afr;
            repeated_control_register<GPIO, BRR, uint32_t, 1, 16> brr;

            template<typename N>
            struct address;
        };

        struct UART final {
            UART() = delete;

            enum class CR : uint32_t {
                RXFFIE = 31,
                TXFEIE = 30,
                FIFOEN = 29,
                M1 = 28,
                M0 = 12,
                EOBIE = 27,
                RTOIE = 26
            };

            control_register<UART, CR, uint32_t> cr1;
            control_register<UART, CR, uint32_t> cr2;
            control_register<UART, CR, uint32_t> cr3;
            control_register<UART, CR, uint32_t> brr;
            control_register<UART, uint8_t> gtpr;
            control_register<UART, uint8_t> rtor;
            control_register<UART, uint8_t> rqr;
            control_register<UART, uint8_t> isr;
            control_register<UART, uint8_t> icr;
            control_register<UART, uint8_t> rdr;
            control_register<UART, uint8_t> tdr;
            control_register<UART, uint8_t> presc;

            template<typename N>
            struct adress;
        } __attribute__((packed));

        // TODO: structure this better somehow
        struct CORDIC {
            using csr_register_type = cordic::cordic_register<CORDIC, cordic::register_types::CSR>;
            using wdata_register_type = cordic::cordic_register<CORDIC, cordic::register_types::WDATA>;
            using rdata_register_type = cordic::cordic_register<CORDIC, cordic::register_types::RDATA>;

            CORDIC() = delete;

            csr_register_type csr;
            wdata_register_type wdata;
            rdata_register_type rdata;

            template<auto N>
            struct address;
        } __attribute__((packed));

        struct AHBENR final {
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

        struct APBENR final {
            enum class APB1ENR1 : uint32_t {
                TIM2EN = (1u << 0u),
                TIM3EN = (1u << 1u),
                TIM4EN = (1u << 2u),
                TIM5EN = (1u << 3u),
                TIM6EN = (1u << 4u),
                TIM7EN = (1u << 5u),
                CRSEN = (1u << 8u),
                RTCAPBEN = (1u << 10u),
                WWDGEN = (1u << 11u),
                SPI2EN = (1u << 14u),
                SP3EN = (1u << 15u),
                USART2EN = (1u << 17u),
                USART3EN = (1u << 18u),
                UART4EN = (1u << 19u),
                UART5EN = (1u << 20u),
                I2C1EN = (1u << 21u),
                I2C2EN = (1u << 22u),
                UCPD1EN = (1u << 23u),
                FDCANEN = (1u << 25u),
                PWREN = (1u << 28u),
                I2C3EN = (1u << 30u),
                LPTIM1EN = (1u << 31u),
            };

            enum class APB1ENR2 : uint32_t { LPUART1EN = (1u << 0u), I2C4EN = (1u << 1u), UCPD1EN = (1u << 8u) };

            enum class APB2ENR : uint32_t {
                SYSCFGEN = (1u << 0u),
                TIM1EN = (1u << 11u),
                SPI1EN = (1u << 12u),
                TIM8EN = (1u << 13u),
                USART1EN = (1u << 14u),
                SPI4EN = (1u << 15u),
                TIM15EN = (1u << 16u),
                TIM16EN = (1u << 17u),
                TIM17EN = (1u << 18u),
                TIM20EN = (1u << 20u),
                SAI1EN = (1u << 21u),
                HRTIM1EN = (1u << 26u),
            };

            control_register<APBENR, APB1ENR1> apb11;
            control_register<APBENR, APB1ENR2> apb12;
            control_register<APBENR, APB2ENR> apb2;
            template<auto N>
            struct address;
        } __attribute__((packed));
    } __attribute__((packed)); /* TODO: Use [[attribute]] Keyword */

    template<>
    struct mcu_info::AHBENR::address<0> {
        static constexpr inline uintptr_t value = mcu_info::base_address::value + 0x48;
    };

    template<>
    struct mcu_info::APBENR::address<0> {
        static constexpr inline uintptr_t value = mcu_info::base_address::value + 0x58;
    };

    template<>
    struct mcu_info::GPIO::address<A> {
        static constexpr inline uintptr_t value = 0x48000000;
    };

    template<>
    struct mcu_info::GPIO::address<B> {
        static constexpr inline uintptr_t value = 0x48000400;
    };

    template<>
    struct mcu_info::GPIO::address<C> {
        static constexpr inline uintptr_t value = 0x48000800;
    };

    template<>
    struct mcu_info::GPIO::address<D> {
        static constexpr inline uintptr_t value = 0x48000C00;
    };

    template<>
    struct mcu_info::GPIO::address<E> {
        static constexpr inline uintptr_t value = 0x48001000;
    };

    template<>
    struct mcu_info::GPIO::address<F> {
        static constexpr inline uintptr_t value = 0x48001400;
    };

    template<>
    struct mcu_info::GPIO::address<G> {
        static constexpr inline uintptr_t value = 0x48001800;
    };

    template<>
    struct mcu_info::UART::adress<uart_nr::one> {
        static constexpr inline uintptr_t value = 0x40013800;
    };

    template<>
    struct mcu_info::UART::adress<uart_nr::two> {
        static constexpr inline uintptr_t value = 0x40004400;
    };

    template<>
    struct mcu_info::CORDIC::address<0> {
        static constexpr inline uintptr_t value = 0x40020C00;
    };

}  // namespace hal::stm::stm32g4

namespace hal::info {
    template<>
    struct vendor_information<hal::stm::stm32g4::mcu_info> {
        static constexpr inline hal::info::vendors vendor = info::vendors::STM;
    };

}  // namespace hal::info

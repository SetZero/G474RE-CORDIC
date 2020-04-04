//
// Created by Sebastian on 31.03.2020.
//

#pragma once

namespace HAL::STM::Cordic {
    enum class register_types { CSR, WDATA, RDATA };

    template<typename Component, register_types reg, typename value_type = uint32_t>
    class cordic_register {};

    template<typename Component>
    struct cordic_register<Component, register_types::CSR, uint32_t> {
        using component_type = Component;
        using bit_type = uint32_t;

        enum class result_amount {
            ONE_REGISTER_VALUE,
            TWO_REGISTER_VALUE
        };

        enum class result_size {
            BIT16,
            BIT32
        };

        /* (31) RRDY: [boolean] */
        [[nodiscard]] bool is_ready() const {
            return hw_register & (1u << 31u);
        }

        /* ARGSIZE: [option: 16bit / 32bit] */
        void inline set_argument_size(result_size size) {
            enable_register<22>(size == result_amount::BIT16);
        }

        /* RESSIZE: [option: 16bit / 32bit] */
        void inline set_result_size(result_size size) {
            enable_register<21>(size == result_size::BIT16);
        }

        /* NARGS: [option: one / two 32bit values] */
        void inline set_argument_amount(result_amount size) {
            enable_register<20>(size == result_amount::DOUBLE);
        }

        /* (19) NRES: [option: one 32bit or two 16bit / two 32bit] */
        void inline set_result_amount(result_amount size) {
            enable_register<19>(size == result_amount::DOUBLE);
        }

        /* (18) DMAWEN: [boolean] */
        void inline enable_dma_write_channel(bool enable) {
            enable_register<18>(enable);
        }

        /* (17) dma: [boolean] */
        void inline enable_dma_read_channel(bool enable) {
            enable_register<17>(enable);
        }

        /* (16) interrupts: [boolean] */
        void inline enable_interrupts(bool enable) {
            enable_register<16>(enable);
        }

        /* TODO: (10:8) SCALE: [number 2^n/2^-n] */

        /* TODO: (7:4) Precision: [number 1 - 15] */

        /* FUNCTION */
        enum class functions {
            cosine = (1u << 0u),
            sine = (1u << 1u),
            phase = (1u << 2u),
            modulus = (1u << 3u),
            arctangent = (1u << 4u),
            hyperbolic_cosine = (1u << 5u),
            hyperbolic_sine = (1u << 6u),
            arctanh = (1u << 7u),
            natural_logarithm = (1u << 8u),
            square_root = (1u << 9u),
        };

        template<functions mode>
        void inline set_function_mode() {
            hw_register = mode & 0xF;
        }

        void inline clear_function_mode() {
            hw_register = hw_register & ~(0xFu);
        }

       private:
        template<uint32_t bit>
        void inline enable_register(bool enable) {
            if(enable)
                hw_register = hw_register | (1u << bit);
            else
                hw_register = hw_register & ~(1u << bit);
        }
        volatile uint32_t hw_register;
    };
}
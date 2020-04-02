//
// Created by Sebastian on 31.03.2020.
//

#pragma once

#include "utils.h"

namespace HAL::STM::Cordic {
    enum class register_types { CSR, WDATA, RDATA };

    template<typename Component, register_types reg, typename value_type = uint32_t>
    class cordic_register {};

    template<typename Component>
    struct cordic_register<Component, register_types::CSR, uint32_t> {
        using component_type = Component;
        using bit_type = uint32_t;

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

        enum class result_amount { ONE_REGISTER_VALUE, TWO_REGISTER_VALUE };

        enum class result_size { BIT16, BIT32 };

        enum class cordic_control_bits : uint8_t {
            ready_flag = 31,
            argsize_flag = 22,
            ressize_flag = 21,
            nargs_flag = 20,
            nres_flag = 19,
            dma_write_enable_flag = 18,
            dma_read_enable_flag = 17,
            interrupt_enable_flag = 16
        };

        /* (31) RRDY: [boolean] */
        [[nodiscard]] bool is_ready() const { return is_flag_set<cordic_control_bits::ready_flag>(hw_register); }

        /* ARGSIZE: [option: 16bit / 32bit] */
        void inline set_argument_size(result_size size) {
            set_bit_flag<cordic_control_bits::argsize_flag>(hw_register, size == result_amount::BIT16);
        }

        /* RESSIZE: [option: 16bit / 32bit] */
        void inline set_result_size(result_size size) {
            set_bit_flag<cordic_control_bits::ressize_flag>(hw_register, size == result_size::BIT16);
        }

        /* NARGS: [option: one / two 32bit values] */
        void inline set_argument_amount(result_amount size) {
            set_bit_flag<cordic_control_bits::nargs_flag>(hw_register, size == result_amount::DOUBLE);
        }

        /* (19) NRES: [option: one 32bit or two 16bit / two 32bit] */
        void inline set_result_amount(result_amount size) {
            set_bit_flag<cordic_control_bits::nres_flag>(hw_register, size == result_amount::DOUBLE);
        }

        /* (18) DMAWEN: [boolean] */
        void inline enable_dma_write_channel(bool enable) {
            set_bit_flag<cordic_control_bits::dma_write_enable_flag>(hw_register, enable);
        }

        /* (17) dma: [boolean] */
        void inline enable_dma_read_channel(bool enable) {
            set_bit_flag<cordic_control_bits::dma_read_enable_flag>(hw_register, enable);
        }

        /* (16) interrupts: [boolean] */
        void inline enable_interrupts(bool enable) {
            set_bit_flag<cordic_control_bits::interrupt_enable_flag>(hw_register, enable);
        }

        template<functions mode>
        void inline set_function_mode() {
            // Make sure only one function is set
            clear_function_mode();
            hw_register &= static_cast<uint8_t>(mode) & 0xFu;
        }

        // TODO: do this differently
        void inline clear_function_mode() { hw_register &= 0x000u; }

        /* TODO: (10:8) SCALE: [number 2^n/2^-n] */

        /* TODO: (7:4) Precision: [number 1 - 15] */

       private:
        volatile uint32_t hw_register;
    };
}  // namespace HAL::STM::Cordic

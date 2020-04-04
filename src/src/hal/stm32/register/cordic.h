//
// Created by Sebastian on 31.03.2020.
//

#pragma once

#include "hal/cordic.h"
#include "hal/cordic_types.h"
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
        enum class functions : uint8_t {
            cosine = 0,
            sine = 1,
            phase = 2,
            modulus = 3,
            arctangent = 4,
            hyperbolic_cosine = 5,
            hyperbolic_sine = 6,
            arctanh = 7,
            natural_logarithm = 8,
            square_root = 9,
        };

        // TODO: add remaining types
        template<CordicHal::functions func>
        static constexpr auto map_function() {
            if constexpr (func == CordicHal::functions::cosine) {
                return functions::cosine;
            }

            return functions::cosine;
        }

        enum class result_amount { ONE_REGISTER_VALUE, TWO_REGISTER_VALUE };

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
        [[nodiscard]] bool is_ready() const { return is_flag_set<cordic_control_bits::ready_flag>(&hw_register); }

        /* ARGSIZE: [option: 16bit / 32bit] */
        void inline set_argument_size(precision size) {
            set_bit_flag<cordic_control_bits::argsize_flag>(&hw_register, size == precision::q1_15);
        }

        /* RESSIZE: [option: 16bit / 32bit] */
        void inline set_result_size(precision size) {
            set_bit_flag<cordic_control_bits::ressize_flag>(&hw_register, size == precision::q1_15);
        }

        /* NARGS: [option: one / two 32bit values] */
        void inline set_argument_amount(result_amount size) {
            set_bit_flag<cordic_control_bits::nargs_flag>(&hw_register, size == result_amount::TWO_REGISTER_VALUE);
        }

        /* (19) NRES: [option: one 32bit or two 16bit / two 32bit] */
        void inline set_result_amount(result_amount size) {
            set_bit_flag<cordic_control_bits::nres_flag>(&hw_register, size == result_amount::TWO_REGISTER_VALUE);
        }

        /* (18) DMAWEN: [boolean] */
        void inline enable_dma_write_channel(bool enable) {
            set_bit_flag<cordic_control_bits::dma_write_enable_flag>(&hw_register, enable);
        }

        /* (17) dma: [boolean] */
        void inline enable_dma_read_channel(bool enable) {
            set_bit_flag<cordic_control_bits::dma_read_enable_flag>(&hw_register, enable);
        }

        /* (16) interrupts: [boolean] */
        void inline enable_interrupts(bool enable) {
            set_bit_flag<cordic_control_bits::interrupt_enable_flag>(&hw_register, enable);
        }

        // TODO: test this
        template<functions mode>
        void inline set_function_mode() {
            // Make sure only one function is set
            hw_register = (hw_register & ~(static_cast<uint32_t>(0xF))) | static_cast<uint32_t>(mode);
        }

        /* TODO: (10:8) SCALE: [number 2^n/2^-n] */
        void inline set_scale(uint8_t scale) {
            hw_register = (hw_register & ~(static_cast<uint32_t>(0x7) << 8)) | static_cast<uint32_t>(scale & 0x7) << 8;
        }

        /* TODO: (7:4) Precision: [number 1 - 15] */
        void inline set_precision(uint8_t precision) {
            hw_register = (hw_register & ~(static_cast<uint32_t>(0xF) << 4)) | static_cast<uint32_t>(precision & 0xF)
                                                                                   << 4;
        }

       private:
        volatile uint32_t hw_register;
    } __attribute__((packed));

    template<typename Component>
    struct cordic_register<Component, register_types::WDATA, uint32_t> {
        template<typename qtype>
        void write_arg(const qtype &arg) {
            // TODO: 16 bit writes
            hw_register = arg.template fixed_point_value<uint32_t>();
        }

       private:
        volatile uint32_t hw_register;
    } __attribute__((packed));

    template<typename Component>
    struct cordic_register<Component, register_types::RDATA, uint32_t> {
        template<typename qtype>
        [[nodiscard]] qtype read_arg() {
            return qtype(hw_register);
        }

       private:
        volatile uint32_t hw_register;
    } __attribute__((packed));

}  // namespace HAL::STM::Cordic

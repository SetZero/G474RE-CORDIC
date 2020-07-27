#pragma once

#include "hal/cordic/cordic_operations.h"
#include "hal/cordic_types.h"
#include "hal/hal_info.h"
#include "hal/register.h"

template<typename MCU, typename PIN>
concept cordic_mcu = stm_mcu<MCU, PIN>;

namespace hal::cordic {
    template<typename CordicNr, cordic_mcu<CordicNr> mcu>
    class cordic {
       public:
        static inline constexpr auto cordic_register = hal::address<typename mcu::CORDIC, CordicNr>;
        using cordic_control_register_type = std::remove_pointer_t<decltype(cordic_register()->csr)>;

        template<typename config, operation_type type, functions function>
        static typename operation<config, type, function>::result_type calculate(
            const operation<config, type, function> &op) {
            using op_type = std::decay_t<decltype(op)>;
            using op_result = typename op_type::result_type;

            cordic_register()
                ->csr.template set_function_mode<cordic_control_register_type::template map_function<function>()>();

            // TODO: configure this differently, maybe most efficient, always use one register value, when q1_15 is used
            cordic_register()->csr.set_argument_size(config::precision);
            cordic_register()->csr.set_result_size(config::precision);
            cordic_register()->csr.set_argument_amount(op_type::num_args);
            cordic_register()->csr.set_result_amount(op_result::num_res);
            cordic_register()->csr.set_precision(static_cast<uint8_t>(config::calculation_precision));
            cordic_register()->csr.set_scale(uint8_t(op.scale()));
            cordic_register()->csr.enable_dma_write_channel(false);
            cordic_register()->csr.enable_dma_read_channel(false);
            cordic_register()->csr.enable_interrupts(false);

            write_args(op);

            op_result result{};
            read_res<op_result>(result, static_cast<uint8_t>(op.scale()));

            return result;
        }

       private:
        template<typename OperationType, std::enable_if_t<static_cast<int>(OperationType::num_args) == 2, int> = 0>
        static void write_args(const OperationType &op) {
            cordic_register()->wdata.write_arg(op.arg1());
            cordic_register()->wdata.write_arg(op.arg2());
        }

        template<typename OperationType, std::enable_if_t<static_cast<int>(OperationType::num_args) == 1, int> = 0>
        static void write_args(const OperationType &op) {
            cordic_register()->wdata.write_arg(op.arg1());
        }

        template<typename OperationResultType,
                 std::enable_if_t<static_cast<int>(OperationResultType::num_res) == 2, int> = 0>
        static void read_res(OperationResultType &result, uint8_t scale) {
            using result_type = typename OperationResultType::result_type;
            while (!cordic_register()->csr.is_ready())
                ;
            result.result(cordic_register()->rdata.template read_arg<result_type>(scale));
            result.secondary_result(cordic_register()->rdata.template read_arg<result_type>(scale));
        }

        template<typename OperationResultType,
                 std::enable_if_t<static_cast<int>(OperationResultType::num_res) == 1, int> = 0>
        static void read_res(OperationResultType &result, uint8_t scale) {
            using result_type = typename OperationResultType::result_type;
            while (!cordic_register()->csr.is_ready())
                ;
            result.result(cordic_register()->rdata.template read_arg<result_type>(scale));
        }
    };
}  // namespace hal::cordic

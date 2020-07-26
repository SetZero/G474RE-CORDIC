#pragma once

#include "hal/cordic_types.h"
#include "hal/hal_info.h"
#include "hal/register.h"

template<typename MCU, typename PIN>
concept cordic_mcu = stm_mcu<MCU, PIN>;

namespace hal::cordic {
    enum struct functions : uint8_t {
        cosine,
        sine,
        phase,
        modulus,
        arctangent,
        hyperbolic_cosine,
        hyperbolic_sine,
        arctanh,
        natural_logarithm,
        square_root
    };

    enum class operation_type { single, pipeline };

    template<precision P>
    class cordic_config final {
       public:
        using qtype = Detail::precision_to_type<P>;

        static inline constexpr auto precision = P;
    };

    template<typename config, operation_type Type, functions Function>
    class operation final {};

    template<typename ResultType, operation_type Type, functions Function>
    class operation_result final {};

    // TODO: consider renaming functions to set args (angle and modulus in this case)
    template<typename Config>
    class operation<Config, operation_type::single, functions::cosine> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::cosine>;
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::cosine>;
        using angle_type = angle<config_type::precision>;

        thiz_type &arg1(angle_type angle) {
            m_angle = angle;
            return *this;
        }

        auto arg1() const { return static_cast<typename config_type::qtype>(m_angle); }

        auto arg2() const { return m_modulus; }

       private:
        angle_type m_angle{0.0f};
        typename config_type::qtype m_modulus{1.0f};
    };

    // TODO: add multiple results
    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::cosine> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::cosine>;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

       private:
        ResultType m_result;
    };

    template<typename CordicNr, cordic_mcu<CordicNr> mcu>
    class cordic {
       public:
        static inline constexpr auto cordic_register = hal::address<typename mcu::CORDIC, CordicNr>;
        using cordic_control_register_type = std::remove_pointer_t<decltype(cordic_register()->csr)>;

        template<typename config, operation_type type, functions function>
        static typename operation<config, type, function>::result_type calculate(
            const operation<config, type, function> &op [[gnu::unused]]) {
            cordic_register()
                ->csr.template set_function_mode<cordic_control_register_type::template map_function<function>()>();

            // TODO: configure this differently, maybe most efficient, always use one register value, when q1_15 is used
            cordic_register()->csr.set_argument_size(config::precision);
            cordic_register()->csr.set_result_size(config::precision);
            cordic_register()->csr.set_argument_amount(cordic_control_register_type::result_amount::TWO_REGISTER_VALUE);
            cordic_register()->csr.set_result_amount(cordic_control_register_type::result_amount::TWO_REGISTER_VALUE);
            cordic_register()->csr.set_precision(uint8_t(10));
            cordic_register()->csr.set_scale(uint8_t(0));
            cordic_register()->csr.enable_dma_write_channel(false);
            cordic_register()->csr.enable_dma_read_channel(false);
            cordic_register()->csr.enable_interrupts(false);

            cordic_register()->wdata.write_arg(op.arg1());
            cordic_register()->wdata.write_arg(op.arg2());

            while (!cordic_register()->csr.is_ready())
                ;

            typename operation<config, type, function>::result_type result{};
            result.result(cordic_register()->rdata.template read_arg<typename config::qtype>());
            volatile auto res [[gnu::unused]] = cordic_register()->rdata.template read_arg<typename config::qtype>();

            return result;
        }

       private:
    };
}  // namespace hal::cordic

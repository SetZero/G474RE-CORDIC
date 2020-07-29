/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 *
 *
 ******************************************************************************
 */

/* Includes */
#include "main.h"

#include <cstdint>
//#include <concepts>
#include "benchmark.h"
#include "hal/cordic.h"
#include "hal/cordic_types.h"
#include "hal/gpio.h"
#include "hal/stm32/stm32g4.h"
#include "hal/stm32/stm32g4/stm32g474re.h"
#include "hal/uart.h"

using namespace units::literals;
namespace mcu_ns = hal::stm::stm32g4;
using used_mcu = mcu_ns::g474re<16'000'000_Hz>;
namespace gpio_values = hal::periphery::gpio_values;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main() {
    init_counter();

    using namespace hal::cordic;
    using port_a = hal::periphery::gpio<mcu_ns::A, used_mcu>;
    using uart_two = hal::periphery::uart<mcu_ns::uart_nr::two, used_mcu>;
    using cordic_one = hal::periphery::cordic<mcu_ns::cordic_nr::one, mcu_ns::mcu_info>;

    cordic_one::init();

    using txpin = port_a::pin<2>;
    using rxpin = port_a::pin<3>;
    uart_two::init<txpin, rxpin, 115200>();

    port_a::set_port_mode<gpio_values::modes::OUTPUT, 5>();

    using cc = cordic_config<precision::q1_31>;
    auto op = create_cordic_operation<cc, functions::cosine>();
    auto op2 = create_cordic_operation<cc, functions::sine>();
    auto op3 = create_cordic_operation<cc, functions::phase>();
    auto op4 = create_cordic_operation<cc, functions::arctangent>();
    auto op5 = create_cordic_operation<cc, functions::modulus>();
    auto op6 = create_cordic_operation<cc, functions::hyperbolic_cosine>();
    auto op7 = create_cordic_operation<cc, functions::hyperbolic_sine>();
    auto op8 = create_cordic_operation<cc, functions::arctanh>();
    auto op9 = create_cordic_operation<cc, functions::natural_logarithm>();
    auto op10 = create_cordic_operation<cc, functions::square_root>();

    int deg = 0;
    while (true) {
        /*port_a::pin<5>::on();
        delay_ms(250);
        port_a::pin<5>::off();
        delay_ms(250);*/
        // while((memory(LPUART_BASE + LPUART_ISR) & (1u << 6u)) >> 6u != 1);

        auto float_val = static_cast<float>(cordic_one::calculate(op).result());
        auto float_val2 = static_cast<float>(cordic_one::calculate(op2).result());

        int16_t rdeg = deg - 180;
        float hyperbolic_argument = rdeg / 180.0f;
        float hyperbolic_argument_atan = rdeg / 370.0f;
        float nat_log_arg = 0.2;
        float sqrt_arg = 2;
        // int atanval = rdeg / 2;
        vec2<precision::q1_31> v;

        op.arg1(angle<precision::q1_31>{degrees{rdeg}});
        op2.arg1(angle<precision::q1_31>{degrees{rdeg}});

        decltype(op4)::args_type::first_arg_type op4_arg;
        decltype(op6)::args_type::first_arg_type op6_arg;

        using setup_benchmark_type = benchmark<decltype(&reset_counter), decltype(&get_counter_value)>;
        setup_benchmark_type b(reset_counter, get_counter_value);
        static constexpr char benchmark_name[] = {"setup arguments"};
        static constexpr char benchmark_name_two[] = {"calculating"};
        static constexpr char benchmark_name_three[] = {"converting back to floats"};

        static constexpr char benchmark_standard_trigon[] = {"calculating with standard trigonmetric functions"};
        uint32_t result = 0;
        uint32_t result_two = 0;
        uint32_t result_three = 0;
        uint32_t gcc_timer = 0;
        {
            auto probe = b.create_probe<benchmark_name>(&result);
            decltype(op8)::args_type::first_arg_type op8_arg{hyperbolic_argument_atan};
            op4_arg = decltype(op4_arg){rdeg / 2.0f};
            op6_arg = decltype(op6_arg){hyperbolic_argument};

            op4.arg1(op4_arg);

            v = vec2<precision::q1_31>{x_coord{float_val}, y_coord{float_val2}};
            op3.arg(v);
            op5.arg(v);
            op6.arg1(op6_arg);
            op7.arg1(op6_arg);
            op8.arg1(op8_arg);
            op9.arg1(decltype(op9)::args_type::first_arg_type{nat_log_arg});
            op10.arg1(decltype(op10)::args_type::first_arg_type{sqrt_arg});
        }

        uart_two::printf<256>("%s took %ld us \r\n", benchmark_name, result);
        typename decltype(op3)::result_type::result_type fixed_val3;
        typename decltype(op5)::result_type::result_type fixed_val4;
        typename decltype(op4)::result_type::result_type fixed_val5;
        typename decltype(op6)::result_type::result_type fixed_val6;
        typename decltype(op7)::result_type::result_type fixed_val7;
        typename decltype(op8)::result_type::result_type fixed_val8;
        typename decltype(op9)::result_type::result_type fixed_val9;
        typename decltype(op10)::result_type::result_type fixed_val10;

        {
            auto probe = b.create_probe<benchmark_name_two>(&result_two);
            fixed_val3 = cordic_one::calculate(op3).result();
            fixed_val4 = cordic_one::calculate(op5).result();
            fixed_val5 = cordic_one::calculate(op4).result();
            fixed_val6 = cordic_one::calculate(op6).result();
            fixed_val7 = cordic_one::calculate(op7).result();
            fixed_val8 = cordic_one::calculate(op8).result();
            fixed_val9 = cordic_one::calculate(op9).result();
            fixed_val10 = cordic_one::calculate(op10).result();
        }

        uart_two::printf<256>("%s took %ld us \r\n", benchmark_name_two, result_two);

        float float_val3{};
        float float_val4{};
        float float_val5{};
        float float_val6{};
        float float_val7{};
        float float_val8{};
        float float_val9{};
        float float_val10{};

        {
            auto probe = b.create_probe<benchmark_name_three>(&result_three);
            float_val3 = static_cast<float>(fixed_val3);
            float_val4 = static_cast<float>(fixed_val4);
            float_val5 = static_cast<float>(fixed_val5);
            float_val6 = static_cast<float>(fixed_val6);
            float_val7 = static_cast<float>(fixed_val7);
            float_val8 = static_cast<float>(fixed_val8);
            float_val9 = static_cast<float>(fixed_val9);
            float_val10 = static_cast<float>(fixed_val10);
        }

        uart_two::printf<256>("%s took %ld us \r\n", benchmark_name_three, result_three);

        float v1{0.0f};
        float v2{0.0f};
        float v3{0.0f};
        float v4{0.0f};
        float v5{0.0f};
        float v6{0.0f};
        float v7{0.0f};
        float v8{0.0f};
        {
            auto probe = b.create_probe<benchmark_standard_trigon>(&gcc_timer);
            v1 = std::cos(rdeg);
            v2 = std::sin(rdeg);
            v3 = std::atan(rdeg);
            v4 = std::cosh(hyperbolic_argument);
            v5 = std::sinh(hyperbolic_argument);
            v6 = std::atanh(hyperbolic_argument_atan);
            v7 = std::log(rdeg);
            v8 = std::sqrt(rdeg);
        }

        uart_two::printf<512>("GCC: %d us vs Cordic: %d\r\n", gcc_timer, result + result_two + result_three);

        uart_two::printf<512>("%d, %d, %d, %d, %d, %d, %d, %d \r\n", static_cast<int>(v1 * 10000),
                              static_cast<int>(v2 * 10000), static_cast<int>(v3 * 10000), static_cast<int>(v4 * 10000),
                              static_cast<int>(v5 * 10000), static_cast<int>(v6 * 10000), static_cast<int>(v7 * 10000),
                              static_cast<int>(v8 * 10000));

        uart_two::printf<512>(
            "cos(%d) * 1000 = %d sin(%d) * 1000 = %d atan2(%d * 1000, %d * 1000) = %d * 10000000 == %d * 10000000 "
            "len(vec) == %d * 1000, scale = %d, value = %d "
            "atan = %d real_atan = %d cosh = %d real_cosh = %d sinh = %d real_sinh = %d atanh = %d real_atanh = %d "
            "logn %d real_logn %d "
            "sqrt %d real_sqrt %d"
            "\r\n",
            rdeg, static_cast<int>(float_val * 1000), rdeg, static_cast<int>(float_val2 * 1000),
            static_cast<int>((float)v.y() * 1000), static_cast<int>((float)v.x() * 1000),
            static_cast<int>(float_val3 * 10000000), static_cast<int>(std::atan2(float_val2, float_val) * 10000000),
            static_cast<int>(float_val4 * 1000), op4_arg.scale(), static_cast<int>(static_cast<float>(op4_arg) * 1000),
            static_cast<int>(float_val5 * static_cast<float>(M_PI) * 100000),
            static_cast<int>(std::atan(rdeg / 2.0f) * 100000),
            static_cast<int>(static_cast<float>(float_val6) * 10000000),
            static_cast<int>(std::cosh(hyperbolic_argument) * 10000000),
            static_cast<int>(static_cast<float>(float_val7) * 10000000),
            static_cast<int>(std::sinh(hyperbolic_argument) * 10000000),
            static_cast<int>(static_cast<float>(float_val8) * 10000000),
            static_cast<int>(std::atanh(hyperbolic_argument_atan) * 10000000),
            static_cast<int>(static_cast<float>(float_val9) * 10000000),
            static_cast<int>(std::log(nat_log_arg) * 10000000),
            static_cast<int>(static_cast<float>(float_val10) * 10000000),
            static_cast<int>(std::sqrt(sqrt_arg) * 10000000));
        deg = (deg + 1) % 360;
        delay_ms(50);
    }
}

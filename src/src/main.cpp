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
#include "hal/cordic.h"
#include "hal/cordic_types.h"
#include "hal/gpio.h"
#include "hal/stm32/stm32g4.h"
#include "hal/stm32/stm32g4/stm32g474re.h"
#include "hal/uart.h"
#include "playground.h"

namespace mcu_ns = hal::stm::stm32g4;
using used_mcu = hal::stm::stm32g4::g474re;
namespace gpio_values = hal::periphery::gpio_values;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main() {
    using namespace hal::cordic;
    using port_a = hal::periphery::gpio<mcu_ns::A, used_mcu>;
    using uart_two = hal::periphery::uart<mcu_ns::uart_nr::two, used_mcu>;
    using cordic_one = hal::cordic::cordic<mcu_ns::cordic_nr::one, mcu_ns::mcu_info>;

    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb1.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB1ENR::CORDIC>();
    hal::address<hal::stm::stm32g4::mcu_info::AHBENR, 0>()
        ->ahb2.add<hal::stm::stm32g4::mcu_info::AHBENR::AHB2ENR::GPIOA>();

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

    int deg = 0;
    while (true) {
        port_a::pin<5>::on();
        delay_ms(250);
        port_a::pin<5>::off();
        delay_ms(250);
        // while((memory(LPUART_BASE + LPUART_ISR) & (1u << 6u)) >> 6u != 1);
        int16_t rdeg = deg - 180;
        float hyperbolic_argument = rdeg / 180.0f;
        float hyperbolic_argument_atan = rdeg / 370.0f;
        float nat_log_arg = 0.2;
        // int atanval = rdeg / 2;
        decltype(op4)::args_type::first_arg_type op4_arg{rdeg / 2.0f};
        decltype(op6)::args_type::first_arg_type op6_arg{hyperbolic_argument};
        decltype(op8)::args_type::first_arg_type op8_arg{hyperbolic_argument_atan};

        op.arg1(angle<precision::q1_31>{degrees{rdeg}});
        op2.arg1(angle<precision::q1_31>{degrees{rdeg}});
        op4.arg1(op4_arg);

        auto float_val = static_cast<float>(cordic_one::calculate(op).result());
        auto float_val2 = static_cast<float>(cordic_one::calculate(op2).result());
        // auto float_op_val = static_cast<float>(op4_arg);
        vec2<precision::q1_31> v{x_coord{float_val}, y_coord{float_val2}};
        op3.arg(v);
        op5.arg(v);
        op6.arg1(op6_arg);
        op7.arg1(op6_arg);
        op8.arg1(op8_arg);
        op9.arg1(decltype(op9)::args_type::first_arg_type{nat_log_arg});

        auto float_val3 = static_cast<float>(cordic_one::calculate(op3).result());
        auto float_val4 = static_cast<float>(cordic_one::calculate(op5).result());
        auto float_val5 = static_cast<float>(cordic_one::calculate(op4).result());
        auto float_val6 = static_cast<float>(cordic_one::calculate(op6).result());
        auto float_val7 = static_cast<float>(cordic_one::calculate(op7).result());
        auto float_val8 = static_cast<float>(cordic_one::calculate(op8).result());
        auto float_val9 = static_cast<float>(cordic_one::calculate(op9).result());
        uart_two::printf<512>(
            "cos(%d) * 1000 = %d sin(%d) * 1000 = %d atan2(%d * 1000, %d * 1000) = %d * 10000000 == %d * 10000000 "
            "len(vec) == %d * 1000, scale = %d, value = %d "
            "atan = %d real_atan = %d cosh = %d real_cosh = %d sinh = %d real_sinh = %d atanh = %d real_atanh = %d "
            "logn %d real_logn %d "
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
            static_cast<int>(std::log(nat_log_arg) * 10000000));
        deg = (deg + 1) % 360;
        delay_ms(50);
    }
}

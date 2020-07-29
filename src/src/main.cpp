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

#include <array>
#include <cstdint>

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

struct benchmark_results {
    uint32_t result_cordic = 0;
    uint32_t result_gcc = 0;

    size_t num_runs = 0;
    float bogus_value = 0.0f;
};

template<hal::cordic::functions Function>
struct gcc_func {};

template<>
struct gcc_func<hal::cordic::functions::cosine> {
    static inline float (*func)(float) = std::cos;
};

template<>
struct gcc_func<hal::cordic::functions::sine> {
    static inline float (*func)(float) = std::sin;
};

template<>
struct gcc_func<hal::cordic::functions::arctanh> {
    static inline float (*func)(float) = std::atan;
};

template<hal::cordic::functions Function>
struct func_input_range {
    static inline constexpr float lower_value = -1.0f;
    static inline constexpr float upper_value = 1.0f;
};

template<>
struct func_input_range<hal::cordic::functions::arctanh> {
    static inline constexpr float lower_value = -127.0f;
    static inline constexpr float upper_value = 127.0f;
};

template<hal::cordic::functions Function>
struct func_info {
    static inline constexpr auto func = Function;
    static inline constexpr auto lower_value = func_input_range<Function>::lower_value;
    static inline constexpr auto upper_value = func_input_range<Function>::upper_value;

    static inline auto gcc_equivalent = gcc_func<Function>::func;
};

template<typename BenchmarkType, typename CordicType, hal::cordic::functions Function, uint32_t num_values = 100>
benchmark_results do_benchmark(const BenchmarkType &benchmark) {
    using namespace hal::cordic;

    using cordic_config = cordic_config<precision::q1_31>;
    using current_func_info = func_info<Function>;
    auto cordic_op [[gnu::unused]] = hal::cordic::create_cordic_operation<cordic_config, Function>();

    std::array<float, num_values> benchmark_values{};
    std::array<decltype(cordic_op), num_values> cordic_benchmark_values{};
    std::array<typename decltype(cordic_op)::result_type, num_values> cordic_benchmark_results{};
    std::array<float, num_values> bogus_values{};
    std::array<float, num_values> second_bogus_values{};
    size_t useable_values = 0;
    float current_value = current_func_info ::lower_value;
    constexpr float value_inc =
        (current_func_info::upper_value - current_func_info::lower_value) / static_cast<float>(num_values);

    for (; useable_values < benchmark_values.size() && current_value < current_func_info::upper_value;
         ++useable_values) {
        benchmark_values[useable_values] = current_value;
        current_value += value_inc;
    }

    static const char cordic_setup_benchmark_name[] = "cordic_setup_bench";
    uint32_t cordic_setup_results = 0;
    {
        auto probe = benchmark.template create_probe<cordic_setup_benchmark_name>(&cordic_setup_results);
        for (auto i = 0u; i < useable_values; ++i) {
            cordic_benchmark_values[i].arg1(
                typename decltype(cordic_op)::args_type::first_arg_type(benchmark_values[i]));
        }
    }

    static const char cordic_calc_benchmark_name[] = "cordic_calc_bench";
    uint32_t cordic_calc_results = 0;
    {
        auto probe = benchmark.template create_probe<cordic_calc_benchmark_name>(&cordic_calc_results);
        for (auto i = 0u; i < useable_values; ++i) {
            cordic_benchmark_results[i] = CordicType::calculate(cordic_benchmark_values[i]);
        }
    }

    static const char cordic_convert_benchmark_name[] = "cordic_convert_bench";
    uint32_t cordic_convert_results = 0;
    {
        auto probe = benchmark.template create_probe<cordic_convert_benchmark_name>(&cordic_convert_results);
        for (auto i = 0u; i < useable_values; ++i) {
            bogus_values[i] = static_cast<float>(cordic_benchmark_results[i].result());
        }
    }

    static const char gcc_benchmark_name[] = "gcc_bench";
    uint32_t gcc_results = 0;
    {
        auto probe = benchmark.template create_probe<gcc_benchmark_name>(&gcc_results);
        for (auto i = 0u; i < useable_values; ++i) {
            second_bogus_values[i] = current_func_info::gcc_equivalent(benchmark_values[i]);
        }
    }

    float total_difference = 0;
    for (auto i = 0u; i < useable_values; ++i) {
        total_difference += bogus_values[i] - second_bogus_values[(useable_values - 1) - i];
    }
    total_difference /= static_cast<float>(useable_values);

    return benchmark_results{.result_cordic = cordic_setup_results + cordic_calc_results + cordic_convert_results,
                             .result_gcc = gcc_results,
                             .num_runs = useable_values,
                             .bogus_value = total_difference};
}

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

    using setup_benchmark_type = benchmark<decltype(&reset_counter), decltype(&get_counter_value)>;
    setup_benchmark_type b(reset_counter, get_counter_value);

    while (true) {
        auto results = do_benchmark<setup_benchmark_type, cordic_one, functions::cosine, 1000>(b);
        uart_two::printf<256>("cos gcc_results: %d, num_runs : %d, complete_value : %d \r\n", results.result_gcc,
                              results.num_runs, static_cast<int>(results.bogus_value * 1000));
        uart_two::printf<256>("cos cordic_results: %d, num_runs : %d, complete_value : %d \r\n", results.result_cordic,
                              results.num_runs, static_cast<int>(results.bogus_value * 1000));

        results = do_benchmark<setup_benchmark_type, cordic_one, functions::sine, 1000>(b);
        uart_two::printf<256>("sine gcc_results: %d, num_runs : %d, complete_value : %d \r\n", results.result_gcc,
                              results.num_runs, static_cast<int>(results.bogus_value * 1000));
        uart_two::printf<256>("sine cordic_results: %d, num_runs : %d, complete_value : %d \r\n", results.result_cordic,
                              results.num_runs, static_cast<int>(results.bogus_value * 1000));

        results = do_benchmark<setup_benchmark_type, cordic_one, functions::arctanh, 1000>(b);
        uart_two::printf<256>("atanh gcc_results: %d, num_runs : %d, complete_value : %d \r\n", results.result_gcc,
                              results.num_runs, static_cast<int>(results.bogus_value * 1000));
        uart_two::printf<256>("atanh cordic_results: %d, num_runs : %d, complete_value : %d \r\n",
                              results.result_cordic, results.num_runs, static_cast<int>(results.bogus_value * 1000));
        delay_ms(1000);
    }
}

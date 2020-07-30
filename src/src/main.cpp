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
    uint32_t bogus_value = 0;
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
struct gcc_func<hal::cordic::functions::hyperbolic_sine> {
    static inline float (*func)(float) = std::sinh;
};

template<>
struct gcc_func<hal::cordic::functions::hyperbolic_cosine> {
    static inline float (*func)(float) = std::cosh;
};

template<>
struct gcc_func<hal::cordic::functions::arctanh> {
    static inline float (*func)(float) = std::atanh;
};

template<>
struct gcc_func<hal::cordic::functions::square_root> {
    static inline float (*func)(float) = std::sqrt;
};

template<>
struct gcc_func<hal::cordic::functions::natural_logarithm> {
    static inline float (*func)(float) = std::log;
};

template<hal::cordic::functions Function>
struct func_input_range {};

template<>
struct func_input_range<hal::cordic::functions::cosine> {
    static inline constexpr float lower_value = -3.140f;
    static inline constexpr float upper_value = 3.140f;
};

template<>
struct func_input_range<hal::cordic::functions::sine> {
    static inline constexpr float lower_value = -3.140f;
    static inline constexpr float upper_value = 3.140f;
};

template<>
struct func_input_range<hal::cordic::functions::hyperbolic_sine> {
    static inline constexpr float lower_value = -1.118f;
    static inline constexpr float upper_value = 1.118f;
};

template<>
struct func_input_range<hal::cordic::functions::hyperbolic_cosine> {
    static inline constexpr float lower_value = -1.118f;
    static inline constexpr float upper_value = 1.118f;
};

template<>
struct func_input_range<hal::cordic::functions::arctanh> {
    static inline constexpr float lower_value = -0.805f;
    static inline constexpr float upper_value = 0.805f;
};

template<>
struct func_input_range<hal::cordic::functions::square_root> {
    static inline constexpr float lower_value = 0.03f;
    static inline constexpr float upper_value = 2.30f;
};

template<>
struct func_input_range<hal::cordic::functions::natural_logarithm> {
    static inline constexpr float lower_value = 0.11f;
    static inline constexpr float upper_value = 9.35f;
};

template<hal::cordic::functions Function>
struct func_info {
    static inline constexpr auto func = Function;
    static inline constexpr auto lower_value = func_input_range<Function>::lower_value;
    static inline constexpr auto upper_value = func_input_range<Function>::upper_value;

    static inline auto gcc_equivalent = gcc_func<Function>::func;
};

template<hal::cordic::functions Function>
struct generate_values {
    template<typename GeneralType, typename BenchmarkValueType, uint32_t num_values>
    static uint32_t call(std::array<GeneralType, num_values> &benchmark_values,
                         std::array<BenchmarkValueType, num_values> &cordic_benchmark_values,
                         BenchmarkValueType init_value, float lower_value, float upper_value, uint8_t quadrant) {
        float quadrant_size = (upper_value - lower_value) / 4.0f;
        float value_inc = quadrant_size / static_cast<float>(num_values);
        float current_value = lower_value + quadrant_size * quadrant;
        uint32_t useable_values = 0;
        for (; useable_values < benchmark_values.size() && current_value < upper_value; ++useable_values) {
            benchmark_values[useable_values] = current_value;
            // It has to be created via the create method !!
            cordic_benchmark_values[useable_values] = init_value;
            current_value += value_inc;
        }

        return useable_values;
    }
};

using uart_two = hal::periphery::uart<mcu_ns::uart_nr::two, used_mcu>;
using port_a = hal::periphery::gpio<mcu_ns::A, used_mcu>;
using cordic_one = hal::periphery::cordic<mcu_ns::cordic_nr::one, mcu_ns::mcu_info>;

template<typename BenchmarkType, typename CordicType, hal::cordic::functions Function, uint32_t num_values = 100>
benchmark_results do_benchmark(const BenchmarkType &benchmark, uint8_t quadrant) {
    using namespace hal::cordic;

    using cordic_config = cordic_config<precision::q1_31, hal::cordic::cordic_algorithm_precision::high>;
    using current_func_info = func_info<Function>;
    auto cordic_op = hal::cordic::create_cordic_operation<cordic_config, Function>();

    std::array<float, num_values> benchmark_values{};
    std::array<decltype(cordic_op), num_values> cordic_benchmark_values{};
    std::array<typename decltype(cordic_op)::result_type, num_values> cordic_benchmark_results{};
    std::array<float, num_values> bogus_values{};
    std::array<float, num_values> second_bogus_values{};

    size_t useable_values = generate_values<Function>::template call<float, decltype(cordic_op), num_values>(
        benchmark_values, cordic_benchmark_values, cordic_op, current_func_info::lower_value,
        current_func_info::upper_value, quadrant);

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
            cordic_benchmark_results[i] = cordic_one::calculate(cordic_benchmark_values[i]);
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

    uint32_t total_difference = 0;
    for (auto i = 0u; i < useable_values; ++i) {
        total_difference +=
            static_cast<decltype(total_difference)>(std::fabs(bogus_values[i] - second_bogus_values[i]) * 1000'000);
        // uart_two::printf<256>("%d %d \t %d \r\n", static_cast<int>(benchmark_values[i] * 1000),
        //                       static_cast<int>(bogus_values[i] * 10000),
        //                       static_cast<int>(second_bogus_values[i] * 10000));
    }

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

    cordic_one::init();

    using txpin = port_a::pin<2>;
    using rxpin = port_a::pin<3>;
    uart_two::init<txpin, rxpin, 115200>();

    port_a::set_port_mode<gpio_values::modes::OUTPUT, 5>();

    using setup_benchmark_type = benchmark<decltype(&reset_counter), decltype(&get_counter_value)>;
    setup_benchmark_type b(reset_counter, get_counter_value);

    while (true) {
        uart_two::printf<256>("============================[BEGIN]============================\r\n");

        for (uint8_t i = 0; i < 4; ++i) {
            uart_two::printf<256>("quadrant : %d \r\n", i);

            auto results = do_benchmark<setup_benchmark_type, cordic_one, functions::cosine, 100>(b, i);
            uart_two::printf<256>("GCC COS \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC COS \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::sine, 100>(b, i);
            uart_two::printf<256>("GCC SIN \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC SIN \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::arctanh, 100>(b, i);
            uart_two::printf<256>("GCC ATANH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC ATANH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::square_root, 100>(b, i);
            uart_two::printf<256>("GCC SQRT \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC SQRT \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::hyperbolic_sine, 100>(b, i);
            uart_two::printf<256>("GCC SINH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC SINH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::hyperbolic_cosine, 100>(b, i);
            uart_two::printf<256>("GCC COSH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC COSH \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::natural_logarithm, 100>(b, i);
            uart_two::printf<256>("GCC LOGN \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n", results.num_runs,
                                  results.bogus_value, results.result_gcc);
            uart_two::printf<256>("CORDIC LOGN \t\t num_runs : %10d, total_difference : %10d run_time :  %10d \r\n",
                                  results.num_runs, results.bogus_value, results.result_cordic);
        }

        uart_two::printf<256>("============================[END]============================\r\n");

        delay_ms(1000);
    }
}

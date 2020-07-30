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

template<>
struct gcc_func<hal::cordic::functions::phase> {
    static inline float (*func)(float, float) = std::atan2;
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

template<>
struct func_input_range<hal::cordic::functions::phase> {
    static inline constexpr float lower_value = -3.141f;
    static inline constexpr float upper_value = 3.141f;
};

template<hal::cordic::functions Function, typename CordicOpType>
struct actions;

template<hal::cordic::functions Function>
struct func_info {
    static inline constexpr auto func = Function;
    static inline constexpr auto lower_value = func_input_range<Function>::lower_value;
    static inline constexpr auto upper_value = func_input_range<Function>::upper_value;

    static inline auto gcc_equivalent = gcc_func<Function>::func;

    template<typename CordicType>
    using action_type = actions<Function, CordicType>;
};

template<hal::cordic::functions Function, typename CordicOpType>
struct actions {
    template<const char benchmark_name[]>
    static inline const auto setup_values = [](auto *counter_value, auto useable_values, const auto &benchmark,
                                               auto &cordic_benchmark_values, const auto &benchmark_values) {
        auto probe = benchmark.template create_probe<benchmark_name>(counter_value);
        for (auto i = 0u; i < useable_values; ++i) {
            cordic_benchmark_values[i].arg1(typename CordicOpType::args_type::first_arg_type(benchmark_values[i]));
        }
    };

    template<const char benchmark_name[]>
    static inline const auto calc_gcc = [](auto *counter_value, auto useable_values, const auto &benchmark,
                                           auto &bogus_values, const auto &benchmark_values) {
        using current_func_info = func_info<Function>;

        auto probe = benchmark.template create_probe<benchmark_name>(counter_value);
        for (auto i = 0u; i < useable_values; ++i) {
            bogus_values[i] = current_func_info::gcc_equivalent(benchmark_values[i]);
        }
    };

    using type = float;
};

template<typename CordicOpType>
struct actions<hal::cordic::functions::phase, CordicOpType> {
    template<const char benchmark_name[]>
    static inline const auto setup_values = [](auto *counter_value, auto useable_values, const auto &benchmark,
                                               auto &cordic_benchmark_values, const auto &benchmark_values) {
        auto probe = benchmark.template create_probe<benchmark_name>(counter_value);
        for (auto i = 0u; i < useable_values; ++i) {
            cordic_benchmark_values[i].arg1(typename CordicOpType::args_type::first_arg_type(x_coord{benchmark_values[i][0]}, y_coord{benchmark_values[i][1]}));
        }
    };

    template<const char benchmark_name[]>
    static inline const auto calc_gcc = [](auto *counter_value, auto useable_values, const auto &benchmark,
                                           auto &bogus_values, const auto &benchmark_values) {
        using current_func_info = func_info<hal::cordic::functions::phase>;

        auto probe = benchmark.template create_probe<benchmark_name>(counter_value);
        for (auto i = 0u; i < useable_values; ++i) {
            bogus_values[i] = current_func_info::gcc_equivalent(benchmark_values[i][1], benchmark_values[i][0]);
        }
    };

    using type = float[2];
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

template<>
struct generate_values<hal::cordic::functions::phase> {
    template<typename GeneralType, typename BenchmarkValueType, uint32_t num_values>
    static uint32_t call(std::array<float[2], num_values> &benchmark_values,
                         std::array<BenchmarkValueType, num_values> &cordic_benchmark_values,
                         BenchmarkValueType init_value, float lower_value, float upper_value, uint8_t quadrant) {
        float quadrant_size = (upper_value - lower_value) / 4.0f;
        float value_inc = quadrant_size / static_cast<float>(num_values);
        float current_value = lower_value + quadrant_size * quadrant;
        uint32_t useable_values = 0;
        for (; useable_values < benchmark_values.size() && current_value < upper_value; ++useable_values) {
            // Generate random vector
            benchmark_values[useable_values][0] = std::sin(current_value);
            benchmark_values[useable_values][1] = std::cos(current_value);
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
    using atype = typename current_func_info::action_type<decltype(cordic_op)>;

    std::array<typename atype::type, num_values> benchmark_values{};
    std::array<decltype(cordic_op), num_values> cordic_benchmark_values{};
    std::array<typename decltype(cordic_op)::result_type, num_values> cordic_benchmark_results{};
    std::array<float, num_values> bogus_values{};
    std::array<float, num_values> second_bogus_values{};

    size_t useable_values = generate_values<Function>::template call<float, decltype(cordic_op), num_values>(
        benchmark_values, cordic_benchmark_values, cordic_op, current_func_info::lower_value,
        current_func_info::upper_value, quadrant);

    static const char cordic_setup_benchmark_name[] = "cordic_setup_bench";
    uint32_t cordic_setup_results = 0;
    atype::template setup_values<cordic_setup_benchmark_name>(&cordic_setup_results, useable_values, benchmark,
                                                              cordic_benchmark_values, benchmark_values);

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
    atype::template calc_gcc<gcc_benchmark_name>(&gcc_results, useable_values, benchmark, second_bogus_values,
                                                 benchmark_values);

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

    static constexpr auto num_runs = 100u;

    while (true) {
        uart_two::printf<256>("============================[BEGIN]============================\r\n");

        for (uint8_t i = 0; i < 4; ++i) {
            uart_two::printf<256>("quadrant : %d \r\n", i);

            uart_two::printf<256>("SUBJ; ALGO; NUM_RUNS; TOTAL_DIFFERENCE; RUN_TIME\r\n");
            auto results = do_benchmark<setup_benchmark_type, cordic_one, functions::cosine, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "COS", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "COS", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::sine, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "SIN", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "SIN", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::arctanh, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "ATANH", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "ATANH", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::square_root, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "SQRT", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "SQRT", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::hyperbolic_sine, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "SINH", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "SINH", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::hyperbolic_cosine, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "COSH", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "COSH", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::natural_logarithm, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "LOGN", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "LOGN", results.num_runs, results.bogus_value,
                                  results.result_cordic);

            results = do_benchmark<setup_benchmark_type, cordic_one, functions::phase, num_runs>(b, i);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "GCC", "ATAN2", results.num_runs, results.bogus_value,
                                  results.result_gcc);
            uart_two::printf<256>("%s, %s, %d, %d, %d \r\n", "CORDIC", "ATAN2", results.num_runs,
            results.bogus_value,
                                  results.result_cordic);
        }

        uart_two::printf<256>("============================[END]============================\r\n");

        delay_ms(1000);
    }
}

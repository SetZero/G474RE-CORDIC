#pragma once

#include <cmath>

#include "hal/cordic_types.h"

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

    struct hyperbolic_bounds {
        static inline constexpr float target_range_upper_bound = 0.559f;
        static inline constexpr float target_range_lower_bound = -0.559f;
    };

    struct hyperbolic_atan_bounds {
        static inline constexpr float target_range_upper_bound = 0.403f;
        static inline constexpr float target_range_lower_bound = -0.403f;
    };

    struct nat_log_bounds {
        static inline constexpr float target_range_upper_bound = 0.875f;
        static inline constexpr float target_range_lower_bound = 0.054f;
    };

    struct sqrt_bounds {
        static inline constexpr float target_range_upper_bound = 0.875f;
        static inline constexpr float target_range_lower_bound = 0.027f;
    };

    enum struct nargs : uint8_t { one = 1, two = 2 };

    enum struct nres : uint8_t { one = 1, two = 2 };

    enum class operation_type { single, pipeline };

    enum class cordic_algorithm_precision : uint8_t { normal = 5u };

    template<typename config, operation_type Type, functions Function>
    class operation final {};

    template<typename ResultType, operation_type Type, functions Function, nres num_result = 1>
    class operation_result final {};

    template<precision P, cordic_algorithm_precision A = cordic_algorithm_precision::normal>
    class cordic_config final {
       public:
        using qtype = Detail::precision_to_type<P>;

        template<typename ScaleType>
        using scaled_qtype = Detail::precision_to_type_with_add<P, Detail::normal_fixed_range, ScaleType>;

        template<typename Range, typename ScaleType>
        using scaled_qtype_with_range = Detail::precision_to_type_with_add<P, Range, ScaleType>;


        static inline constexpr auto precision = P;
        static inline constexpr auto calculation_precision = A;
    };

    template<typename FirstArgType, typename SecondArgType = void>
    struct general_operation_args final {
        using first_arg_type = FirstArgType;
        using second_arg_type = SecondArgType;

        static inline constexpr nargs num_args = nargs::two;
        first_arg_type m_arg1;
        second_arg_type m_arg2;
    };

    template<typename FirstArgType>
    struct general_operation_args<FirstArgType, void> final {
        using first_arg_type = FirstArgType;
        using second_arg_type = void;

        static inline constexpr nargs num_args = nargs::one;

        first_arg_type m_arg1;
    };

    template<typename FirstResType, typename SecondResType = void>
    struct general_operation_res final {
        using first_res_type = FirstResType;
        using second_res_type = SecondResType;

        static inline constexpr uint8_t num_res = 1 + !std::is_same_v<SecondResType, void>;
    };

    template<typename Config, operation_type Type, functions Function, typename GeneralOperationArgs,
             typename GeneralOperationResult, nres num_res = nres::one>
    class general_operation final {
       public:
        using config_type = Config;
        using thiz_type = general_operation<Config, Type, Function, GeneralOperationArgs, GeneralOperationResult>;
        using args_type = GeneralOperationArgs;
        using res_type = GeneralOperationResult;

        static inline constexpr auto num_args = args_type::num_args;
        static inline constexpr auto function = Function;

        using result_type = operation_result<typename config_type::qtype, operation_type::single, Function, num_res>;

        thiz_type &arg1(const typename args_type::first_arg_type &arg1) {
            m_args.m_arg1 = arg1;
            return *this;
        }

        template<typename T, std::enable_if_t<!std::is_convertible_v<T, typename args_type::second_arg_type> ||
                                                  std::is_same_v<T, typename args_type::second_arg_type>,
                                              int> = 0>
        thiz_type &arg2(const T &value) {
            m_args.m_arg2 = value;
            return *this;
        }

        constexpr auto arg1() const { return m_args.m_arg1; }

        constexpr auto arg2() const { return m_args.m_arg2; }

        constexpr uint8_t scale() const { return m_args.m_arg1.scale(); }

       private:
        args_type m_args;
    };

    template<typename Config, functions Function>
    struct create_op_helper {
        using type = operation<Config, operation_type::single, Function>;

        static inline constexpr auto create() { return type{}; }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::cosine> {
        using type = general_operation<Config, operation_type::single, functions::cosine,
                                       general_operation_args<angle<Config::precision>, typename Config::qtype>,
                                       general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            res.arg2(typename Config::qtype{1.0f});
            return res;
        }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::sine> {
        using type = general_operation<Config, operation_type::single, functions::sine,
                                       general_operation_args<angle<Config::precision>, typename Config::qtype>,
                                       general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            res.arg2(typename Config::qtype{1.0f});
            return res;
        }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::hyperbolic_cosine> {
        using argument_type =
            typename Config::scaled_qtype_with_range<hyperbolic_bounds, scales<hyperbolic_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::hyperbolic_cosine,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::hyperbolic_sine> {
        using argument_type =
            typename Config::scaled_qtype_with_range<hyperbolic_bounds, scales<hyperbolic_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::hyperbolic_sine,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::arctanh> {
        using argument_type =
            typename Config::scaled_qtype_with_range<hyperbolic_atan_bounds, scales<hyperbolic_atan_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::arctanh, general_operation_args<argument_type>,
                              general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::arctangent> {
        using argument_type = typename Config::scaled_qtype<
            scales<normal_bounds, std::integer_sequence<unsigned int, 0, 1, 2, 3, 4, 5, 6, 7>>>;

        using type =
            general_operation<Config, operation_type::single, functions::arctangent,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    struct natural_logarithm_scales {
        static inline constexpr std::array ranges{
            Detail::range{.upper_bound = 1.0f - std::numeric_limits<float>::min(), .lower_bound = 0.107f, .scale = 1},
            Detail::range{.upper_bound = 3.0f - std::numeric_limits<float>::min(), .lower_bound = 1.0f, .scale = 2},
            Detail::range{.upper_bound = 7.0f - std::numeric_limits<float>::min(), .lower_bound = 3.0f, .scale = 3},
            Detail::range{.upper_bound = 9.35f, .lower_bound = 7.0f, .scale = 4}};

        static inline constexpr std::array<float, 4> prepared_inversed_scales{
            1.0f / constexpr_pow(2.0f, 1u), 1.0f / constexpr_pow(2.0f, 2u), 1.0f / constexpr_pow(2.0f, 3u),
            1.0f / constexpr_pow(2.0f, 4u)};
    };

    template<typename Config>
    struct create_op_helper<Config, functions::natural_logarithm> {
        using argument_type = typename Config::scaled_qtype_with_range<nat_log_bounds, natural_logarithm_scales>;

        using type =
            general_operation<Config, operation_type::single, functions::natural_logarithm,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    struct sqrt_scales {
        static inline constexpr std::array ranges{
            Detail::range{.upper_bound = 0.75f - std::numeric_limits<float>::min(), .lower_bound = 0.027f, .scale = 0},
            Detail::range{.upper_bound = 1.75f - std::numeric_limits<float>::min(), .lower_bound = 0.75f, .scale = 1},
            Detail::range{.upper_bound = 2.341f, .lower_bound = 1.75f, .scale = 2}};

        static inline constexpr std::array<float, 3> prepared_inversed_scales{
            1.0f / constexpr_pow(2.0f, 0u), 1.0f / constexpr_pow(2.0f, 1u), 1.0f / constexpr_pow(2.0f, 2u)};
    };

    template<typename Config>
    struct create_op_helper<Config, functions::square_root> {
        using argument_type = typename Config::scaled_qtype_with_range<sqrt_bounds, sqrt_scales>;

        using type =
            general_operation<Config, operation_type::single, functions::square_root,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        static inline constexpr auto create() { return type{}; }
    };

    template<typename Config, functions Function>
    static inline constexpr auto create_cordic_operation() {
        return create_op_helper<Config, Function>::create();
    }

    template<typename Config>
    class operation<Config, operation_type::single, functions::phase> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::phase>;
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::phase, nres::one>;

        static inline constexpr auto function = functions::phase;
        static inline constexpr auto num_args = nargs::two;

        thiz_type &arg(const vec2<config_type::precision> &v) {
            m_v = v;
            return *this;
        }

        auto arg1() const { return m_v.x(); }

        auto arg2() const { return m_v.y(); }

        auto scale() const { return 0u; }

       private:
        vec2<config_type::precision> m_v{};
    };

    template<typename Config>
    class operation<Config, operation_type::single, functions::modulus> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::modulus>;
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::modulus, nres::one>;

        static inline constexpr auto function = functions::modulus;
        static inline constexpr auto num_args = nargs::two;

        thiz_type &arg(const vec2<config_type::precision> &v) {
            m_v = v;
            return *this;
        }

        auto arg1() const { return m_v.x(); }

        auto arg2() const { return m_v.y(); }

        auto scale() const { return 0u; }

       private:
        vec2<config_type::precision> m_v{};
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::cosine, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::cosine, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::sine, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::sine, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::phase, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::phase, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            m_result.soft_scale(M_PI);
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::modulus, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::modulus, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            m_secondary_result.soft_scale(M_PI);
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::arctangent, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::arctangent, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

       private:
        ResultType m_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::hyperbolic_cosine, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::hyperbolic_cosine, res>;

        static inline constexpr auto num_res = res;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType, nres res>
    class operation_result<ResultType, operation_type::single, functions::hyperbolic_sine, res> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::hyperbolic_sine, res>;

        static inline constexpr auto num_res = nres::two;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        thiz_type &secondary_result(ResultType result) {
            m_secondary_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::arctanh, nres::one> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::arctanh, nres::one>;

        static inline constexpr auto num_res = nres::one;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

       private:
        ResultType m_result;
    };

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::natural_logarithm, nres::one> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::natural_logarithm, nres::one>;

        static inline constexpr auto num_res = nres::one;

        thiz_type &result(ResultType result) {
            m_result = result;
            m_result.soft_scale(2);

            return *this;
        }

        result_type result() const { return m_result; }

       private:
        ResultType m_result;
    };

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::square_root, nres::one> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::square_root, nres::one>;

        static inline constexpr auto num_res = nres::one;

        thiz_type &result(ResultType result) {
            m_result = result;
            return *this;
        }

        result_type result() const { return m_result; }

       private:
        ResultType m_result;
    };

}  // namespace hal::cordic

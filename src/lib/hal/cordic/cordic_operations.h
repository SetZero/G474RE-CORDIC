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

    enum struct nargs : uint8_t { one = 1, two = 2 };

    enum struct nres : uint8_t { one = 1, two = 2 };

    enum class operation_type { single, pipeline };

    enum class cordic_algorithm_precision : uint8_t { normal = 6u };

    template<typename config, operation_type Type, functions Function>
    class operation final {};

    template<typename ResultType, operation_type Type, functions Function>
    class operation_result final {};

    template<precision P, cordic_algorithm_precision A = cordic_algorithm_precision::normal>
    class cordic_config final {
       public:
        using qtype = Detail::precision_to_type<P>;

        template<typename ScaleType>
        using scaled_qtype = Detail::precision_to_type_with_add<P, Detail::normal_fixed_range, ScaleType>;

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
             typename GeneralOperationResult>
    class general_operation final {
       public:
        using config_type = Config;
        using thiz_type = general_operation<Config, Type, Function, GeneralOperationArgs, GeneralOperationResult>;
        using args_type = GeneralOperationArgs;
        using res_type = GeneralOperationResult;

        static inline constexpr auto num_args = args_type::num_args;
        static inline constexpr auto function = Function;

        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::cosine>;

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
            typename Config::scaled_qtype<scales<Detail::hyperbolic_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::hyperbolic_cosine,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            return res;
        }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::hyperbolic_sine> {
        using argument_type =
            typename Config::scaled_qtype<scales<Detail::hyperbolic_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::hyperbolic_sine,
                              general_operation_args<argument_type>, general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            return res;
        }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::arctanh> {
        using argument_type = typename Config::scaled_qtype<
            scales<Detail::hyperbolic_atan_bounds, std::integer_sequence<unsigned int, 1>>>;

        using type =
            general_operation<Config, operation_type::single, functions::arctanh, general_operation_args<argument_type>,
                              general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            return res;
        }
    };

    template<typename Config>
    struct create_op_helper<Config, functions::arctangent> {
        using argument_type = typename Config::scaled_qtype<
            scales<Detail::normal_bounds, std::integer_sequence<unsigned int, 0, 1, 2, 3, 4, 5, 6, 7>>>;

        using type =
            general_operation<Config, operation_type::single, functions::arctangent, general_operation_args<argument_type>,
                              general_operation_res<typename Config::qtype>>;

        // Preset modulus to 1.0f
        static inline constexpr auto create() {
            type res{};
            return res;
        }
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
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::phase>;

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
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::modulus>;

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

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::cosine> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::cosine>;

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
    class operation_result<ResultType, operation_type::single, functions::sine> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::sine>;

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
    class operation_result<ResultType, operation_type::single, functions::phase> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::phase>;

        static inline constexpr auto num_res = nres::two;

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

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::modulus> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::modulus>;

        static inline constexpr auto num_res = nres::two;

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

    template<typename ResultType>
    class operation_result<ResultType, operation_type::single, functions::arctangent> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::arctangent>;

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
    class operation_result<ResultType, operation_type::single, functions::hyperbolic_cosine> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::hyperbolic_cosine>;

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
    class operation_result<ResultType, operation_type::single, functions::hyperbolic_sine> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::hyperbolic_sine>;

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
    class operation_result<ResultType, operation_type::single, functions::arctanh> final {
       public:
        using result_type = ResultType;
        using thiz_type = operation_result<ResultType, operation_type::single, functions::arctanh>;

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

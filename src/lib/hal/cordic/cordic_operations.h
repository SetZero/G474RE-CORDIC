#pragma once

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

    enum struct nargs : uint8_t { one, two };

    enum struct nres : uint8_t { one, two };

    enum class operation_type { single, pipeline };

    enum class cordic_algorithm_precision : uint8_t { normal = 6u };

    template<precision P, cordic_algorithm_precision A = cordic_algorithm_precision::normal>
    class cordic_config final {
       public:
        using qtype = Detail::precision_to_type<P>;

        template<typename ScaleType>
        using scaled_qtype = Detail::precision_to_type_with_add<P, Detail::normal_fixed_range, ScaleType>;

        static inline constexpr auto precision = P;
        static inline constexpr auto calculation_precision = A;
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

        static inline constexpr auto num_args = nargs::two;

        thiz_type &arg1(angle_type angle) {
            m_angle = angle;
            return *this;
        }

        auto arg1() const { return static_cast<typename config_type::qtype>(m_angle); }

        auto arg2() const { return m_modulus; }

        auto scale() const { return 0u; }

       private:
        angle_type m_angle{0.0f};
        typename config_type::qtype m_modulus{1.0f};
    };

    template<typename Config>
    class operation<Config, operation_type::single, functions::sine> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::sine>;
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::sine>;
        using angle_type = angle<config_type::precision>;

        static inline constexpr auto num_args = nargs::two;

        thiz_type &arg1(angle_type angle) {
            m_angle = angle;
            return *this;
        }

        auto arg1() const { return static_cast<typename config_type::qtype>(m_angle); }

        auto arg2() const { return m_modulus; }

        auto scale() const { return 0u; }

       private:
        angle_type m_angle{0.0f};
        typename config_type::qtype m_modulus{1.0f};
    };

    template<typename Config>
    class operation<Config, operation_type::single, functions::phase> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::phase>;
        using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::phase>;

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
    class operation<Config, operation_type::single, functions::arctangent> final {
       public:
        using config_type = Config;
        using thiz_type = operation<Config, operation_type::single, functions::arctangent>;
        using result_type =
            operation_result<typename config_type::qtype, operation_type::single, functions::arctangent>;
        // TODO create bounds type
        using argument_type = typename config_type::scaled_qtype<scales<Detail::normal_bounds, std::integer_sequence<unsigned int, 0, 1, 2, 3, 4, 5, 6, 7>>>;

        static inline constexpr auto num_args = nargs::two;

        thiz_type &arg1(const argument_type &arg) {
            m_arg = arg;
            return *this;
        }

        auto arg1() const { return m_arg; }

        auto scale() const { return m_arg.scale(); }

       private:
        argument_type m_arg{};
    };

    // TODO: add multiple results
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
            return *this;
        }

        result_type result() const { return m_result; }

        result_type secondary_result() const { return m_secondary_result; }

       private:
        ResultType m_result;
        ResultType m_secondary_result;
    };

}  // namespace hal::cordic

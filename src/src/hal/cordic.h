#pragma once

#include "hal/cordic_types.h"

// TODO: add needed functions
enum class functions { cosine, sine };

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

template<typename Config>
class operation<Config, operation_type::single, functions::cosine> final {
   public:
    using config_type = Config;
    using thiz_type = operation<Config, operation_type::single, functions::cosine>;
    using result_type = operation_result<typename config_type::qtype, operation_type::single, functions::cosine>;

    thiz_type &arg1(angle<config_type::precision> a [[gnu::unused]]) { return *this; }
};

template<typename ResultType>
class operation_result<ResultType, operation_type::single, functions::cosine> final {
   public:
    using result_type = ResultType;

    result_type result() const { return result_type(0.0f); }
};

template<typename CordicRegister>
class cordic {
   public:
    using cordic_register = CordicRegister;

    template<typename config, operation_type type, functions function>
    typename operation<config, type, function>::result_type calculate(const operation<config, type, function> &op
                                                                      [[gnu::unused]]) {
        return typename operation<config, type, function>::result_type{};
    }
};

#pragma once

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <utility>

enum struct precision : uint8_t { q1_15, q1_31 };

namespace Detail {
    struct range {
        const float upper_bound;
        const float lower_bound;
        const unsigned int scale;

        constexpr inline auto contains(float value) const { return upper_bound > value && lower_bound < value; }
    };

    struct normal_bounds {
        static inline constexpr float target_range_upper_bound = 1.0f;
        static inline constexpr float target_range_lower_bound = -1.0f;
    };
}  // namespace Detail

template<typename bounds, typename IndexType, auto... Ints>
struct scales {};

// TODO: add specialization for only one scale
template<typename bounds, auto FirstInt, decltype(FirstInt)... Ints>
struct scales<bounds, std::integer_sequence<decltype(FirstInt), FirstInt, Ints...>> {
    using bound_type = bounds;

    static inline constexpr auto calc_range(unsigned int scale) {
        return Detail::range{.upper_bound = static_cast<float>(std::pow<float>(2, static_cast<int>(scale))) *
                                            bounds::target_range_upper_bound,
                             .lower_bound = static_cast<float>(std::pow<float>(2, static_cast<int>(scale))) *
                                            bounds::target_range_lower_bound,
                             .scale = scale};
    }

    static inline constexpr std::array ranges{calc_range(FirstInt), calc_range(Ints)...};

    static inline constexpr auto find_smallest_scale(float value) {
        for (const auto &current_range : ranges) {
            if (current_range.contains(value)) {
                return current_range.scale;
            }
        }

        return ranges[ranges.size() - 1].scale;
    }
};

namespace Detail {
    template<uint8_t all_bits>
    using internal_type = std::conditional_t<
        (all_bits <= 8), int8_t,
        std::conditional_t<(all_bits <= 16), int16_t, std::conditional_t<(all_bits <= 32), int32_t, int64_t>>>;

    struct normal_fixed_range {
        static inline constexpr double lower_bound = -1.0f;
        static inline constexpr double upper_bound = 1.0f;
    };

    // TODO: fix negative numbers
    template<uint8_t integer_bit, uint8_t fractional_bit, typename allowed_range, typename scales_lookup>
    class q_number {
        using type = internal_type<integer_bit + fractional_bit>;
        static inline constexpr uint8_t sign_pos = sizeof(type) * CHAR_BIT - 1;
        static inline constexpr auto lower_bound = allowed_range::lower_bound;
        static inline constexpr auto upper_bound = allowed_range::upper_bound;

        static_assert(integer_bit >= 0, "There has to be at least one bit for the sign in this implementation");

       private:
        template<typename T>
        constexpr void set_to_value(T value) {
            m_scale = scales_lookup::find_smallest_scale(value);
            value *= static_cast<T>(std::pow(2.0, -m_scale));
            value = std::clamp<T>(value, lower_bound, upper_bound);
            m_value = static_cast<type>(std::round(value * static_cast<type>(std::pow(2, fractional_bit))));
        }

       public:
        constexpr q_number() = default;
        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        constexpr explicit q_number(T value) {
            set_to_value(value);
        }

        // TODO: own type to initialize with fixed integer, otherwise when calling constructor with a regular integer, meant as normal number this can be error prone
        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        constexpr explicit q_number(T value, uint8_t scale = 0) {
            m_value = value;
            m_scale = scale;
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        q_number &operator=(T value) {
            set_to_value(value);

            return *this;
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        constexpr explicit operator T() const {
            return static_cast<type>(m_value) * static_cast<T>(powf(2.0f, -fractional_bit)) * static_cast<T>(std::pow(2, m_scale));
        }

        template<typename T, std::enable_if_t<sizeof(T) >= sizeof(type) && std::is_unsigned_v<T>, int> = 0>
        constexpr T fixed_point_value() const {
            return T(m_value);
        }

        constexpr auto scale() const { return m_scale; }

       private:
        type m_value = 0;
        uint8_t m_scale = 0;
    };

}  // namespace Detail

using normal_scales = scales<Detail::normal_bounds, std::integer_sequence<unsigned int, 0>>;

template<uint8_t integer_bit, uint8_t fractional_bit, typename bounds = Detail::normal_fixed_range,
         typename allowed_scales = normal_scales>
struct q_fixed_helper {
    using type = Detail::q_number<integer_bit, fractional_bit, bounds, allowed_scales>;
};

using q1_15 = typename q_fixed_helper<1, 15>::type;
using q1_31 = typename q_fixed_helper<1, 31>::type;

template<typename range>
using q1_15_ranged = typename q_fixed_helper<1, 15, range, normal_scales>::type;

template<typename range>
using q1_31_ranged = typename q_fixed_helper<1, 31, range, normal_scales>::type;

template<typename ScalesType>
using q1_15_with_scales = typename q_fixed_helper<1, 15, Detail::normal_fixed_range, ScalesType>::type;

template<typename ScalesType>
using q1_31_with_scales = typename q_fixed_helper<1, 31, Detail::normal_fixed_range, ScalesType>::type;

namespace Detail {
    template<precision p>
    using precision_to_type = std::conditional_t<p == precision::q1_15, q1_15, q1_31>;

    template<precision p, typename... Types>
    using precision_to_type_with_add =
        std::conditional_t<p == precision::q1_15, typename q_fixed_helper<1, 15, Types...>::type,
                           typename q_fixed_helper<1, 31, Types...>::type>;

    template<precision p>
    using float_type = std::conditional_t<p == precision::q1_15, float, double>;

}  // namespace Detail

// TODO: convert to class to also do a radians class
enum struct degrees : int16_t {};

template<precision p, typename range>
class ranged_angle {
   public:
    using type = Detail::precision_to_type_with_add<p, range>;

    // TODO: discuss how to spot too large numbers
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    constexpr explicit ranged_angle(const T &value) : m_value(value / static_cast<T>(M_PI)) {}

    constexpr ranged_angle(degrees d) : m_value(Detail::float_type<p>(d) / Detail::float_type<p>(180.0)) {}

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    constexpr explicit operator T() const {
        return T(static_cast<T>(m_value) * T(M_PI));
    }

    // TODO: add output to degrees
    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value;
};

template<precision p>
using angle = ranged_angle<p, Detail::normal_fixed_range>;

struct x_coord {
    float value = 0;
};

struct y_coord {
    float value = 0;
};

x_coord operator"" _x(long double value) { return x_coord{static_cast<float>(value)}; }

y_coord operator"" _y(long double value) { return y_coord{static_cast<float>(value)}; }

template<precision p>
class vec2 {
   public:
    using type = Detail::precision_to_type<p>;

    constexpr vec2(const x_coord &x = 0.0_x, const y_coord &y = 0.0_y) {
        if (std::fabs(x.value) >= 1.0f || std::fabs(y.value) >= 1.0f) {
            m_soft_scale = std::max<decltype(x.value)>(std::fabs(x.value), std::fabs(y.value)) * 1.15f;
            m_x = x.value / m_soft_scale;
            m_y = y.value / m_soft_scale;
        } else {
            m_x = x.value;
            m_y = y.value;
        }
    }

    constexpr auto x() const { return m_x; }

    constexpr auto y() const { return m_y; }

    constexpr auto soft_scale() const { return m_soft_scale; }

   private:
    q1_31 m_x;
    q1_31 m_y;
    float m_soft_scale = 1.0f;
};

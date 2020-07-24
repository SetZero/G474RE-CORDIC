#pragma once

#include <climits>
#include <cmath>
#include <cstdint>
#include <type_traits>

enum struct precision : uint8_t { q1_15, q1_31 };

namespace Detail {

    template<uint8_t all_bits>
    using internal_type = std::conditional_t<
        (all_bits <= 8), int8_t,
        std::conditional_t<(all_bits <= 16), int16_t, std::conditional_t<(all_bits <= 32), int32_t, int64_t>>>;

    // TODO: fix negative numbers
    template<uint8_t integer_bit, uint8_t fractional_bit>
    class q_number {
        using type = internal_type<integer_bit + fractional_bit>;
        static inline constexpr uint8_t sign_pos = sizeof(type) * CHAR_BIT - 1;

        // TODO: fix this
        static_assert(integer_bit >= 0, "There has to be at least one bit for the sign in this implementation");

       public:
        constexpr q_number() = default;
        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        constexpr explicit q_number(T value) {
            m_value = static_cast<type>(std::round(value * static_cast<type>(std::pow(2, fractional_bit))));
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        constexpr explicit q_number(T value) {
            m_value = value;
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        q_number &operator=(T value) {
            m_value = static_cast<type>(std::round(value * static_cast<type>(std::pow(2, fractional_bit))));
            return *this;
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        constexpr explicit operator T() const {
            return static_cast<type>(m_value) * powf(2.0f, -fractional_bit);
        }

        template<typename T, std::enable_if_t<sizeof(T) >= sizeof(type) && std::is_unsigned_v<T>, int> = 0>
        constexpr T fixed_point_value() const {
            return T(m_value);
        }

       private:
        type m_value = 0;
    };

}  // namespace Detail

using q1_15 = Detail::q_number<1, 15>;
using q1_31 = Detail::q_number<1, 31>;

namespace Detail {
    template<precision p>
    using precision_to_type = std::conditional_t<p == precision::q1_15, q1_15, q1_31>;

    template<precision p>
    using float_type = std::conditional_t<p == precision::q1_15, float, double>;

}  // namespace Detail

// TODO: convert to class to also do a radians class
enum struct degrees : int16_t {};

template<precision p>
class angle {
   public:
    using type = Detail::precision_to_type<p>;

    // TODO: discuss how to spot too large numbers
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    constexpr explicit angle(const T &value) : m_value(value / static_cast<T>(M_PI)) {}

    constexpr angle(degrees d) : m_value(Detail::float_type<p>(d) / Detail::float_type<p>(180.0)) {}

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
class modulus {
   public:
    using type = Detail::precision_to_type<p>;

    template<typename T>
    constexpr explicit modulus(const T &value) {
        if (value >= 1) {
            m_scale = value * Detail::float_type<p>(1.5);
            m_value = value * std::pow(m_scale, -1);
        } else {
            m_value = value;
        }
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    constexpr explicit operator T() const {
        return T(m_value) * static_cast<T>(m_scale);
    }

    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value{};
    Detail::float_type<p> m_scale{1.0};
};

// TODO: maybe add valid range to this class (as type for example)
template<precision p>
class hyperbolic_angle {
   public:
    using type = Detail::precision_to_type<p>;

    template<typename T>
    explicit hyperbolic_angle(const T &value [[gnu::unused]]) {}

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    explicit operator T() const {
        return T(m_value);
    }

    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value;
};


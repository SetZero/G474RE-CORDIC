#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

enum struct precision : uint8_t { q1_15, q1_31 };

namespace Detail {

template <uint8_t all_bits>
using internal_type = std::conditional_t < all_bits < 8,
      int8_t,
      std::conditional_t <
          all_bits<16, int16_t,
                   std::conditional_t<all_bits<32, int32_t, int64_t>>>;

template <uint8_t integer_bit, uint8_t fractional_bit>
class q_number {
    using type = internal_type<integer_bit + fractional_bit>;

   public:
    constexpr q_number() = default;
    template <typename T,
              std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    constexpr explicit q_number(T value) {
        m_value =
            static_cast<type>(std::round(value * std::pow(2, fractional_bit)));
    }

    template <typename T,
              std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    q_number &operator=(T value) {
        m_value =
            static_cast<type>(std::round(value * std::pow(2, fractional_bit)));
        return *this;
    }

    template <typename T,
              std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    constexpr explicit operator T() const {
        return T(m_value * std::pow(T(2), -fractional_bit));
    }

   private:
    type m_value = 0;
};

}  // namespace Detail

using q1_15 = Detail::q_number<1, 15>;
using q1_31 = Detail::q_number<1, 31>;

namespace Detail {
template <precision p>
using precision_to_type =
    std::conditional_t<p == precision::q1_15, q1_15, q1_31>;

template <precision p>
using float_type = std::conditional_t<p == precision::q1_15, float, double>;

}  // namespace Detail

template <precision p>
class angle {
   public:
    using type = Detail::precision_to_type<p>;

    template <typename T>
    constexpr explicit angle(const T &value)
        : m_value(value / static_cast<T>(M_PI)) {}

    template <typename T>
    constexpr explicit operator T() const {
        return T(static_cast<T>(m_value) * static_cast<T>(M_PI));
    }

    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value;
};

template <precision p>
class modulus {
   public:
    using type = Detail::precision_to_type<p>;

    template <typename T>
    constexpr explicit modulus(const T &value) {
        if (value > T(1)) {
            m_scale = value * Detail::float_type<p>(1.5);
            m_value = value * std::pow(m_scale, -1);
        } else {
            m_value = value;
        }
    }

    template <typename T>
    constexpr explicit operator T() const {
        return T(m_value) * static_cast<T>(m_scale);
    }

    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value{};
    Detail::float_type<p> m_scale{1.0};
};

// TODO: maybe add valid range to this class (as type for example)
template <precision p>
class hyperbolic_angle {
   public:
    using type = Detail::precision_to_type<p>;

    template <typename T>
    explicit hyperbolic_angle(const T &value [[gnu::unused]]) {}

    template <typename T>
    explicit operator T() const {
        return T(m_value);
    }

    constexpr explicit operator type() const { return m_value; }

   private:
    type m_value;
};

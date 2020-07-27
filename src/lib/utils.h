#pragma once

#include <hal/hal_info.h>

#include <algorithm>
#include <array>
#include <climits>
#include <concepts>
#include <cstdint>
#include <tuple>
#include <utility>
#include <type_traits>

template<auto bit, typename T>
void set_bit(T *value) {
    *value = *value | (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void unset_bit(T *value) {
    *value = *value & ~(1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void toggle_bit(T *value) {
    *value = *value ^ (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
[[nodiscard]] bool is_flag_set(const T *value) {
    return *value & (T(1) << static_cast<T>(bit));
}

template<auto bit, typename T>
void set_bit_flag(T *value, bool flag) {
    unset_bit<bit, T>(value);
    *value = *value | (flag << static_cast<uint8_t>(bit));
}

template<auto Left, decltype(Left) Right, decltype(Left)... Positions>
constexpr auto range_check() {
    const std::array positions{Positions...};

    return std::all_of(positions.cbegin(), positions.cend(),
                       [](const auto &current_value) { return current_value >= Left && current_value <= Right; });
    ;
}

template<auto Left, decltype(Left) Right, decltype(Left)... Positions>
concept are_all_in_range = range_check<Left, Right, Positions...>();

template<auto FirstType, decltype(FirstType)... Others>
struct type_of_numbers {
    using type = decltype(FirstType);
};

template<auto... Numbers>
using type_of_numbers_t = typename type_of_numbers<Numbers...>::type;

template<auto... Others>
using make_integer_sequence = std::integer_sequence<type_of_numbers_t<Others...>, Others...>;

template<std::unsigned_integral T, typename BitsetType, auto... Bits>
requires(are_all_in_range<0, sizeof(T) * CHAR_BIT - 1, Bits...>) constexpr auto set_bits(
    BitsetType value_to_set, T init_value = {0}, make_integer_sequence<Bits...> sequence = {}) {
    (void)sequence;
    std::array bitpositions{Bits...};
    T res{init_value};

    for (std::size_t i = 0; i < bitpositions.size(); ++i) {
        res &= ~(1 << bitpositions[i]);
        res |= value_to_set[i] << bitpositions[i];
    }

    return res;
}

template<std::unsigned_integral T, typename BitsetType, auto... Bits>
requires(are_all_in_range<0, sizeof(T) * CHAR_BIT - 1, Bits...>) auto get_bits(
    BitsetType value_to_get_from, make_integer_sequence<Bits...> sequence = {}) {
    (void)sequence;
    std::array bitpositions{Bits...};

    T res{0};

    for (std::size_t i = 0; i < bitpositions.size(); ++i) {
        res |= value_to_get_from[bitpositions[i]] << i;
    }

    return res;
}

template<typename ValueType1, typename ValueType2, uint8_t MappingSize>
struct value_mapper {
    template<typename... ValuePairs>
    constexpr value_mapper(ValuePairs... args) : m_mappings{args...} {}

    constexpr ValueType2 operator[](ValueType1 index) const {
        auto result = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                                   [&index](auto &current_value) { return current_value.first == index; });

        return (*result).second;
    }

    const std::array<std::pair<ValueType1, ValueType2>, MappingSize> m_mappings;
};

template<typename... ValuePairs>
value_mapper(ValuePairs... args)
    -> value_mapper<typename std::tuple_element_t<0, std::tuple<ValuePairs...>>::first_type,
                    typename std::tuple_element_t<0, std::tuple<ValuePairs...>>::second_type, sizeof...(args)>;
template<bool...>
struct bool_pack;

template<bool... bs>
static inline constexpr bool all_true = std::is_same_v<bool_pack<bs..., true>, bool_pack<true, bs...>>;

void delay_ms(uint32_t n) {
    for (; n > 0; n--)
        for (uint32_t i = 0; i < 3195; i++) asm("nop");
}

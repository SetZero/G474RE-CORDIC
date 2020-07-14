#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include <utility>

template<auto bit, typename T>
void set_bit(T value) {
    value |= (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void unset_bit(T value) {
    value &= ~(1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void toggle_bit(T value) {
    value ^= (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
[[nodiscard]] bool is_flag_set(T value) {
    return value & (T(1) << static_cast<T>(bit));
}

template<auto bit, typename T>
void set_bit_flag(T value, bool flag) {
    unset_bit<bit, T>(value);
    value |= (flag << static_cast<uint8_t>(bit));
}

template<typename ValueType1, typename ValueType2, uint8_t MappingSize>
struct value_mapper {
    template<typename... ValuePairs>
    constexpr value_mapper(ValuePairs... args) : m_mappings{args...} {}

    // TODO: implement
    constexpr ValueType2 operator[](ValueType1 index [[gnu::unused]]) const { return m_mappings[0].second; }

    std::array<std::pair<ValueType1, ValueType2>, MappingSize> m_mappings;
};

template<typename... ValuePairs>
value_mapper(ValuePairs... args)
    ->value_mapper<typename std::tuple_element_t<0, std::tuple<ValuePairs...>>::first_type,
                   typename std::tuple_element_t<0, std::tuple<ValuePairs...>>::second_type, sizeof...(args)>;


template<typename MCU, typename PIN>
concept gpio_mcu =
requires {
    typename MCU::GPIO::template address<PIN>; // needs GPIO
};
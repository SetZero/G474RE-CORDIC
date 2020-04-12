#pragma once

#include <cstdint>

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

#pragma once

#include <cstdint>

template<auto bit, typename T>
void set_bit_flag(volatile T &value, bool flag) {
    value &= (flag << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void set_bit(volatile T &value) {
    value |= (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void unset_bit(volatile T &value) {
    value &= ~(1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
void toggle_bit(volatile T &value) {
    value ^= (1u << static_cast<uint8_t>(bit));
}

template<auto bit, typename T>
[[nodiscard]] bool is_flag_set(volatile T &value) {
    return value & (1u << static_cast<uint8_t>(bit));
}

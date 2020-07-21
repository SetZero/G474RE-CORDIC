#pragma once

#include <hal/hal_info.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <tuple>
#include <utility>
#include <concepts>

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

template<typename T>
concept input_register_type = requires(T reg) {
    { reg.clear() };
};

template<typename T>
concept output_register_type = requires(T reg) {
    { *reg } -> std::integral;
};

template<typename MCU, typename PIN>
concept stm_mcu = requires(MCU::GPIO a) {
    typename MCU::GPIO::template address<PIN>;
    requires input_register_type<decltype(a.moder)>;
    requires input_register_type<decltype(a.otyper)>;
    requires input_register_type<decltype(a.ospeedr)>;
    requires input_register_type<decltype(a.pupdr)>;
    requires output_register_type<decltype(a.idr)>;
    requires input_register_type<decltype(a.bssr_set_io)>;
    requires input_register_type<decltype(a.bssr_clear_io)>;
    requires input_register_type<decltype(a.afr)>;
    requires input_register_type<decltype(a.moder)>;
};

template<typename MCU, typename PIN>
concept gpio_mcu = stm_mcu<MCU, PIN>;

template<typename MCU>
concept mcu_with_vendor_info = requires {
    MCU::vendor_information::vendors;                                                // needs Vendor Name
    std::is_same_v<decltype(MCU::vendor_information::vendors), hal::info::vendors>;  // vendors must be a vendor :P
};

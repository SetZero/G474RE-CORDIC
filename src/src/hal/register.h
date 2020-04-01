//
// Created by Sebastian on 30.03.2020.
//

#pragma once
#include <type_traits>

namespace HAL {
    template<typename Component, typename byte_type, typename ValueType = uint32_t>
    struct control_register final {
        using component_type = Component;
        using value_type = ValueType;
        using bit_type = byte_type;

        control_register() = delete;
        control_register(const control_register&) = delete;
        control_register(control_register&&) = delete;
        control_register& operator=(const control_register&) = delete;
        control_register& operator=(control_register&&) = delete;

        template<typename... T>
        void inline set(T... v) {
            static_assert((std::is_same<T, byte_type>::value& ...));
            hw_register = (static_cast<value_type>(v) | ...);
        }

        template<byte_type F>
        void inline add()
        {
            hw_register |= static_cast<value_type>(F);
        }

       private:
        volatile value_type hw_register;
    };

    template<typename Component, auto N>
    [[nodiscard]] constexpr inline auto& address() {
        return *reinterpret_cast<Component*>(Component::template address<N>::value);
    }
}

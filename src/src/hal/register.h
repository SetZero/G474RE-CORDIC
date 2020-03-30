//
// Created by Sebastian on 30.03.2020.
//

#pragma once
#include <type_traits>

namespace HAL {
    template<typename Component, typename BitType, typename ValueType = uint32_t>
    struct ControlRegister final {
        using component_type = Component;
        using value_type = ValueType;
        using bit_type = BitType;

        ControlRegister() = delete;
        ControlRegister(const ControlRegister&) = delete;
        ControlRegister(ControlRegister&&) = delete;
        ControlRegister& operator=(const ControlRegister&) = delete;
        ControlRegister& operator=(ControlRegister&&) = delete;

        template<typename... T>
        void inline set(T... v) {
            static_assert((std::is_same<T, BitType>::value& ...));
            hwRegister = (static_cast<value_type>(v) | ...);
        }

        template<BitType F>
        void inline add()
        {
            hwRegister |= static_cast<value_type>(F);
        }

       private:
        volatile value_type hwRegister;
    };

    template<typename Component>
    [[nodiscard]] constexpr inline auto& address() {
        return *reinterpret_cast<Component*>(Component::address::value);
    }
}

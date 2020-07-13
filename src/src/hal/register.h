//
// Created by Sebastian on 30.03.2020.
//

#pragma once

#include <cstdint>
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
            static_assert((std::is_same<T, byte_type>::value & ...));
            hw_register = (static_cast<value_type>(v) | ...);
        }

        template<byte_type F>
        void inline add() {
            hw_register = hw_register | static_cast<value_type>(F);
        }

       private:
        volatile value_type hw_register;
    } __attribute__((packed));

    template<typename Component, typename byte_type, typename ValueType = uint32_t, uint32_t bit_width = 1,
             uint32_t bits = std::numeric_limits<ValueType>::digits>
    struct repeated_control_register final {
        using component_type = Component;
        using value_type = ValueType;
        using bit_type = byte_type;

        repeated_control_register() = delete;
        repeated_control_register(const repeated_control_register&) = delete;
        repeated_control_register(repeated_control_register&&) = delete;
        repeated_control_register& operator=(const repeated_control_register&) = delete;
        repeated_control_register& operator=(repeated_control_register&&) = delete;

        template<value_type position, byte_type F>
        void inline add() {
            static_assert(position <= values);
            hw_register = hw_register | (static_cast<value_type>(F) << (bit_width * position));
        }

        template<value_type position>
        void inline clear() {
            static_assert(position <= values, "Index out of Range");
            hw_register = hw_register & ~(((1u << bit_width) - 1) << (bit_width * position));
        }

        template<value_type position, byte_type F>
        void inline clear_add() {
            clear<position>();
            add<position, F>();
        }

       private:
        static constexpr inline auto values = bits / bit_width;
        volatile value_type hw_register;
    } __attribute__((packed));

    enum class data_register_type : uint32_t { READ_WRITE, READ_ONLY, RESERVED };

    template<typename component, data_register_type Mode = data_register_type::RESERVED, typename ValueType = uint32_t,
             ValueType mask = ValueType{0xFFFF'FFFF}>
    struct data_register;

    template<typename Component, typename ValueType, ValueType mask>
    struct data_register<Component, data_register_type::RESERVED, ValueType, mask> final {
        typedef Component component_type;
        typedef ValueType value_type;
        data_register() = delete;
        data_register(const data_register&) = delete;
        data_register(data_register&&) = delete;
        data_register& operator=(const data_register&) = delete;
        data_register& operator=(data_register&&) = delete;

       private:
        volatile value_type hwRegister;
    };
    template<typename Component, typename ValueType, ValueType mask>
    struct data_register<Component, data_register_type::READ_ONLY, ValueType, mask> final {
        typedef Component component_type;
        typedef ValueType value_type;
        data_register() = delete;
        data_register(const data_register&) = delete;
        data_register(data_register&&) = delete;
        data_register& operator=(const data_register&) = delete;
        data_register& operator=(data_register&&) = delete;
        [[nodiscard]] inline value_type operator*() const { return hwRegister & mask; }

       private:
        volatile value_type hwRegister;
    };

    template<typename Component, typename ValueType, ValueType mask>
    struct data_register<Component, data_register_type::READ_WRITE, ValueType, mask> final
    {
        typedef Component component_type;
        typedef ValueType value_type;
        data_register() = delete;
        data_register(const data_register&) = delete;
        data_register(data_register&&) = delete;
        data_register& operator=(const data_register&) = delete;
        data_register& operator=(data_register&&) = delete;

        inline volatile value_type& operator*()
        {
            return hwRegister;
        }
        [[nodiscard]] inline value_type operator*() const
        {
            return hwRegister & mask;
        }
       private:
        volatile value_type hwRegister;
    };

    template<typename Component, auto N>
    [[nodiscard]] constexpr inline auto address() {
        return reinterpret_cast<Component*>(Component::template address<N>::value);
    }

    template<typename Component, typename N>
    [[nodiscard]] constexpr inline auto address() {
        return reinterpret_cast<Component*>(Component::template address<N>::value);
    }
}  // namespace HAL

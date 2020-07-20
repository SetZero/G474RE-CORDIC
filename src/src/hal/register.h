//
// Created by Sebastian on 30.03.2020.
//

#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace hal {
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

        template<byte_type F, value_type... position>
        void inline set() {
            static_assert(all_true<(position <= values)...>);
            hw_register = ((static_cast<value_type>(F) << (bit_width * position)) | ...);
        }

        template<byte_type F, value_type... position>
        void inline add() {
            static_assert(all_true<(position <= values)...>);
            hw_register = hw_register | ((static_cast<value_type>(F) << (bit_width * position)) | ...);
        }

        template<value_type... position>
        void inline clear() {
            static_assert(all_true<(position <= values)...>, "Index out of Range");
            hw_register = hw_register & (~(((value_type{1} << bit_width) - 1) << (bit_width * position)) & ...);
        }

        template<byte_type F, value_type... position>
        void inline clear_add() {
            clear<position...>();
            add<F, position...>();
        }

       private:
        static constexpr inline value_type values = bits / bit_width;
        volatile value_type hw_register;
    } __attribute__((packed));

    template<typename T>
    concept RegisterDescription = requires {
        typename T::position_type;
        typename T::data_type;
        typename T::enum_type;
        T::function;
    };

    template<typename T>
    concept BitPositionType = requires(T a) {
        typename T::position_type;
        T::contains_pos(T::position_type(0u));
    };

    template<typename... Values>
    constexpr bool is_set(Values... values) {
        const std::array arr{values...};

        return std::all_of(arr.cbegin(), arr.cend(), [&arr](auto& current_value) {
            return std::count(arr.cbegin(), arr.cend(), current_value) == 1;
        });
    }

    template<auto... BitPos>
    concept valid_positions = (std::unsigned_integral<decltype(BitPos)> && ...) && (is_set(BitPos...));

    template<typename IndexType>
    struct bit_pos_detail;

    template<auto FirstPos, auto... BitPos>
    requires(valid_positions<FirstPos, BitPos...>) struct bit_pos_detail<
        std::integer_sequence<decltype(FirstPos), FirstPos, (BitPos)...>> {
        static inline constexpr auto num_pos = sizeof...(BitPos) + 1;
        static inline constexpr std::array bitpos{FirstPos, BitPos...};

        template<auto... T>
        static inline constexpr auto contains_pos() {
            is_set<T..., FirstPos, BitPos...>();
        }
    };

    template<auto FirstPos, decltype(FirstPos)... BitPos>
    using bit_pos = bit_pos_detail<std::integer_sequence<decltype(FirstPos), FirstPos, BitPos...>>;

    template<auto Offset, typename IndexType>
    struct range_to_positions_helper;

    template<auto Offset, decltype(Offset)... BitPos>
    struct range_to_positions_helper<Offset, std::integer_sequence<decltype(Offset), BitPos...>> {
        using type = bit_pos<Offset, (Offset + 1) + BitPos...>;
    };

    template<auto Offset, typename IndexType>
    using range_to_positions_helper_t = range_to_positions_helper<Offset, IndexType>::type;

    template<auto Start, auto End>
    using bit_range = range_to_positions_helper_t<
        Start, std::make_integer_sequence<decltype(Start), static_cast<decltype(Start)>(End - Start)>>;

    template<auto Function, typename DataType, typename PositionType>
    struct register_entry_desc {
        using enum_type = decltype(Function);
        // TODO: add data_type concept
        using data_type = DataType;
        using position_type = PositionType;

        static inline constexpr auto function = Function;
    };

    template<typename EnumType, EnumType Function, RegisterDescription... Description>
    requires(is_set(Description::function...)) constexpr size_t lookup_function_index() {
        const std::array functions{Description::function...};

        return std::distance(functions.cbegin(), std::find(functions.cbegin(), functions.cend(), Function));
    }

    template<typename RegisterType, RegisterDescription... Description>
    requires(is_set(Description::function...) && std::integral<RegisterType>) struct register_desc {
        using description_container_t = std::tuple<Description...>;
        using register_type = RegisterType;
        using enum_type = std::tuple_element_t<0, description_container_t>::enum_type;

        template<enum_type Function>
        using get_type_to_function =
            std::tuple_element_t<lookup_function_index<enum_type, Function, Description...>(), description_container_t>;

        template<enum_type Function>
        using data_type = typename get_type_to_function<Function>::data_type;

        template<enum_type Function>
        static inline constexpr auto start_bit = get_type_to_function<Function>::start;

        template<enum_type Function>
        static inline constexpr auto end_bit = get_type_to_function<Function>::end;

        template<enum_type Function>
        void set_value(data_type<Function> value) {
            std::bitset<std::min(sizeof(value) * 4ul, 64ul)> set(value);
            using current_description = get_type_to_function<Function>;
            const auto num_pos = current_description::position_type::num_pos;

            for (std::remove_const_t<decltype(num_pos)> i = 0; i < num_pos; ++i) {
                const auto current_bitpos = current_description::position_type::bitpos[i];

                m_register = m_register & ~(1 << current_bitpos);
                m_register = m_register | (set[i] << current_bitpos);
            }
        }

        template<enum_type Function>
        data_type<Function> get_value() {
            std::bitset<64ul> set;
            using current_description = get_type_to_function<Function>;
            const auto num_pos = current_description::position_type::num_pos;

            for (std::remove_const_t<decltype(num_pos)> i = 0; i < num_pos; ++i) {
                const auto current_bitpos = current_description::position_type::bitpos[i];

                set[i] = (m_register >> current_bitpos) & 1;
            }

            // TODO: maybe do this differently ?
            return static_cast<data_type<Function>>(set.to_ullong());
        }

       private:
        register_type m_register;
    };
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
    struct data_register<Component, data_register_type::READ_WRITE, ValueType, mask> final {
        typedef Component component_type;
        typedef ValueType value_type;
        data_register() = delete;
        data_register(const data_register&) = delete;
        data_register(data_register&&) = delete;
        data_register& operator=(const data_register&) = delete;
        data_register& operator=(data_register&&) = delete;

        inline volatile value_type& operator*() { return hwRegister; }
        [[nodiscard]] inline value_type operator*() const { return hwRegister & mask; }

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
}  // namespace hal

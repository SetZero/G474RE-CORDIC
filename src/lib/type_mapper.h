#pragma once

#include <tuple>
#include <type_traits>

template<typename T, typename Head, typename... Types>
struct count_occurences;

template<typename T, typename Head, typename... Types>
struct count_occurences<T, std::tuple<Head, Types...>> {
    static constexpr inline unsigned int occ = std::is_same_v<T, Head> + count_occurences<T, std::tuple<Types...>>::occ;
};

template<typename T, typename Head>
struct count_occurences<T, std::tuple<Head>> {
    static constexpr inline unsigned int occ = std::is_same_v<T, Head>;
};

template<typename TupleType, typename N>
struct count_eq_types {
    using current_type = std::tuple_element_t<N::value, TupleType>;
    using next_index = std::integral_constant<unsigned int, N::value - 1>;

    static constexpr inline unsigned int value =
        count_occurences<current_type, TupleType>::occ + count_eq_types<TupleType, next_index>::value;
};

template<typename TupleType>
struct count_eq_types<TupleType, std::integral_constant<unsigned int, 0>> {
    static constexpr inline unsigned int value = count_occurences<std::tuple_element_t<0, TupleType>, TupleType>::occ;
};

template<typename T, typename Head, typename... Types>
struct find_type {
    using next_type = typename find_type<T, Types...>::type;
    using type = std::conditional_t<std::is_same_v<T, typename Head::type>, Head, next_type>;
};

template<typename T, typename Head>
struct find_type<T, Head> {
    using type = std::conditional_t<std::is_same_v<T, typename Head::type>, Head, void>;
};

template<typename... Types>
concept unique_types =
    count_eq_types<std::tuple<Types...>, std::integral_constant<unsigned int, sizeof...(Types) - 1>>::value ==
    sizeof...(Types);

template<typename T, auto Value>
struct type_value_pair {
    using type = T;

    static inline auto constexpr value = Value;
};

template<typename... Types>
requires(unique_types<typename Types::type...>) struct type_mapper {
    using tuple_type = std::tuple<Types...>;

    constexpr type_mapper(Types...) {}

    template<typename T>
    constexpr auto lookup_type() {
        return find_type<T, Types...>::type::value;
    }
};

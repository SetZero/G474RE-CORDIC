#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "utils.h"

TEST_CASE("Utils test") {
    uint32_t value = 3;
    uint32_t value2 = 2;
    auto result = set_bits<uint32_t, std::bitset<32>, 5, 6>(value);
    std::bitset<32> to_test{result};
    REQUIRE(uint32_t(to_test[6] << 1 | to_test[5]) == value);

    auto result2 = set_bits<uint32_t, std::bitset<32>, 31, 6>(value2);
    std::bitset<32> to_test2{result2};
    REQUIRE(uint32_t(to_test2[6] << 1 | to_test2[31]) == value2);

    auto result3 = get_bits<uint32_t, std::bitset<32>, 5, 6>(to_test);
    REQUIRE(result3 == value);

    auto result4 = get_bits<uint32_t, std::bitset<32>, 31, 6>(to_test2);
    REQUIRE(result4 == value2);

    static_assert(std::is_same_v<type_of_numbers_t<1, 2, 3>, int>);
    static_assert(std::is_same_v<make_integer_sequence<5u, 6u, 1u>, std::integer_sequence<unsigned int, 5, 6, 1>>);
}

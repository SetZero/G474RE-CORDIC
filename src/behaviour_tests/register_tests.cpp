#define CATCH_CONFIG_MAIN
#include <bitset>
#include <cstdint>

#include "catch.hpp"
#include "hal/register.h"

enum class functions { enable, scale, read, reserved, reserved2, split };

enum class values {};

TEST_CASE("Basic Register tests") {
    using namespace hal;

    using enable_type = register_entry_desc<functions::enable, bool, bit_pos<31u>>;
    using scale_type = register_entry_desc<functions::scale, uint8_t, bit_range<28u, 30u>>;
    using read_only_type = register_entry_desc<functions::read, uint8_t, bit_range<24u, 27u>, access_mode::read_only>;
    using reserved_type = register_entry_desc<functions::reserved, reserved_type, bit_range<0u, 9u>>;
    using reserved2_type = register_entry_desc<functions::reserved2, reserved_type, bit_range<11u, 22u>>;
    using split_type = register_entry_desc<functions::split, uint8_t, bit_pos<10u, 23u>>;
    using register_type = register_desc<volatile uint32_t, enable_type, scale_type, read_only_type, reserved_type,
                                        reserved2_type, split_type>;

    volatile uint32_t value;
    register_type *r1 = new ((void *)&value) register_type{};

    r1->set_value<functions::enable>(true);
    r1->set_value<functions::scale>(uint8_t(4));
    r1->set_value<functions::split>(uint8_t(3));

    std::bitset<32> to_test{value};
    REQUIRE(to_test[31] == true);
    REQUIRE(((to_test[30] << 2) | (to_test[29] << 1) | to_test[28]) == uint8_t(4));
    REQUIRE(((to_test[23] << 1) | (to_test[10])) == uint8_t(3));

    r1->set_value<functions::enable>(false);
    r1->set_value<functions::scale>(uint8_t(3));
    r1->set_value<functions::split>(uint8_t(1));

    std::bitset<32> second_test{value};
    REQUIRE(second_test[31] == false);
    REQUIRE(((second_test[30] << 2) | (second_test[29] << 1) | second_test[28]) == uint8_t(3));
    REQUIRE(((second_test[23] << 1) | (second_test[10])) == uint8_t(1));

    REQUIRE(r1->get_value<functions::enable>() == false);
    REQUIRE(r1->get_value<functions::scale>() == 3);
    REQUIRE(r1->get_value<functions::split>() == 1);
}

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstdint>
#include <bitset>

#include "hal/register.h"

enum class functions { enable , scale };

enum class values {  };

TEST_CASE("Basic Register tests") {
    using namespace hal;

    using enable_type = register_entry_desc<functions::enable, bool, bit_pos<31u>>;
    using scale_type = register_entry_desc<functions::scale, uint8_t, bit_range<28u, 30u>>;
    using register_type = register_desc<volatile uint32_t, enable_type, scale_type>;

    volatile uint32_t value;
    register_type *r1 = new ((void *)&value) register_type{};

    r1->set_value<functions::enable>(true);
    r1->set_value<functions::scale>(uint8_t(4));

    std::bitset<32> to_test{value};
    REQUIRE(to_test[31] == true);
    REQUIRE(((to_test[30] << 2) | (to_test[29] << 1) | to_test[28]) == uint8_t(4));

    r1->set_value<functions::enable>(false);
    r1->set_value<functions::scale>(uint8_t(3));

    std::bitset<32> second_test{value};
    REQUIRE(second_test[31] == false);
    REQUIRE(((second_test[30] << 2) | (second_test[29] << 1) | second_test[28]) == uint8_t(3));
}

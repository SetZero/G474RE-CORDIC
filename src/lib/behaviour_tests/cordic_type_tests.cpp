#define CATCH_CONFIG_MAIN
#include <cmath>

#include "catch.hpp"
#include "hal/cordic_types.h"

TEST_CASE("Fixed precision types") {
    auto q1_float = 0.05f;
    q1_15 q1(q1_float);

    REQUIRE(std::fabs(static_cast<float>(q1)) - std::fabs(q1_float) < 0.0005);

    auto q2_float = 0.0124354;
    q1_31 q2(q2_float);

    REQUIRE(std::fabs(static_cast<double>(q2)) - std::fabs(q2_float) < 0.0005);

    auto q3_float = -1.0;
    q1_31 q3(q3_float);

    REQUIRE(std::fabs(static_cast<double>(q3)) - std::fabs(q3_float) < 0.0005);
}

TEST_CASE("Other Types") { 
    REQUIRE(static_cast<double>(angle<precision::q1_31>(degrees{180})) - M_PI < 0.0005); 
    REQUIRE(static_cast<double>(angle<precision::q1_31>(degrees{75})) - (75 / 180.0f * M_PI) < 0.0005); 
    auto fixed_value = angle<precision::q1_31>(degrees{75});
    REQUIRE(static_cast<double>(fixed_value) - ((75 / 180.0f * M_PI) / M_PI) < 0.0005); 
}

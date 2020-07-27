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

    auto fvalue = 5.0f;
    auto mod = modulus<precision::q1_31>(fvalue);
    REQUIRE(static_cast<double>(mod) - fvalue < 0.005f);

    auto negfvalue = 5.0f;
    auto negmod = modulus<precision::q1_31>(negfvalue);
    REQUIRE(static_cast<double>(negmod) - negfvalue < 0.005f);

    float x_val = -0.5f;
    float y_val = 0.5f;
    vec2f<precision::q1_31> v{x_coord{x_val}, y_coord{y_val}};

    REQUIRE(std::fabs(static_cast<double>(v.x())) - std::fabs(x_val) < 0.005f);
    REQUIRE(std::fabs(static_cast<double>(v.y())) - std::fabs(y_val) < 0.005f);

    float bigger_x_val = -5;
    float bigger_y_val = -7;
    vec2f<precision::q1_31> bigger_v{x_coord{bigger_x_val}, y_coord{bigger_y_val}};

    REQUIRE(std::fabs(static_cast<double>(bigger_v.x()) * bigger_v.soft_scale()) - std::fabs(bigger_x_val) < 0.005f);
    REQUIRE(std::fabs(static_cast<double>(bigger_v.y()) * bigger_v.soft_scale()) - std::fabs(bigger_y_val) < 0.005f);
}

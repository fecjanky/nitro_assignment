#include "nitro/fwd.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <nitro/geometry.hpp>
#include <nitro/io.hpp>

TEST_CASE("test rectangle api", "[geometry][basic]")
{

    SECTION("basic construction")
    {
        nitro::rectangle r { { .x = 123, .y = 24 }, 10, 40 };
        REQUIRE(r.origin().x == 123);
        REQUIRE(r.origin().y == 24);
        REQUIRE(r.width() == 10);
        REQUIRE(r.height() == 40);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    }
    SECTION("invalid parameters")
    {
        REQUIRE_THROWS(nitro::rectangle { { -2, -124 }, 0, 1 });
        REQUIRE_THROWS(nitro::rectangle { { -2, -124 }, 1, 0 });
    }
}

TEST_CASE("test rectangle from json text", "[io][basic]")
{
    const auto rect_json = R"-({"x": 100, "y": 200, "w": 250, "h": 80 })-"_json;
    const auto rect      = nitro::to_rectangle(rect_json);
    REQUIRE(rect.origin().x == 100);
    REQUIRE(rect.origin().y == 200);
    REQUIRE(rect.width() == 250);
    REQUIRE(rect.height() == 80);
}

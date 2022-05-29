#include "test_utils.hpp"
#include <bits/ranges_algo.h>
#include <compare>
#include <cstddef>
#include <nitro/fwd.hpp>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <nitro/geometry.hpp>
#include <nitro/io.hpp>

#include <type_traits>

TEST_CASE("test rectangle api", "[geometry][basic]")
{

    SECTION("basic construction")
    {
        nitro::rectangle r { { .x = 123, .y = 24 }, { 10, 40 }, id(321) };
        REQUIRE(r.origin().x == 123);
        REQUIRE(r.origin().y == 24);
        REQUIRE(r.width() == 10);
        REQUIRE(r.height() == 40);
        REQUIRE(r.id() == 321);
        REQUIRE(r.parent() == nullptr);
        SECTION("with parent")
        {
            nitro::rectangle r2 { { .x = 123, .y = 34 }, { 10, 20 }, &r };
            REQUIRE(r2.origin().x == 123);
            REQUIRE(r2.origin().y == 34);
            REQUIRE(r2.width() == 10);
            REQUIRE(r2.height() == 20);
            REQUIRE(r2.id() == 321);
            REQUIRE(r2.parent() == &r);
        }
        SECTION("Comparison")
        {
            nitro::rectangle r_copy { { .x = 123, .y = 24 }, { 10, 40 }, id(321) };
            nitro::rectangle r_mod_parent { { .x = 123, .y = 24 }, { 10, 40 }, &r };
            nitro::rectangle r2 { { .x = 123, .y = 24 }, { 10, 40 }, id(322) };
            nitro::rectangle r3 { { .x = 123, .y = 24 }, { 10, 41 }, id(321) };

            REQUIRE(std::is_eq(r <=> r));
            REQUIRE(std::is_eq(r <=> r_copy));
            REQUIRE(r.id() == r_mod_parent.id());
            REQUIRE(std::is_neq(r <=> r_mod_parent));
            REQUIRE(std::is_neq(r <=> r2));
            REQUIRE(std::is_neq(r <=> r3));
        }
    }
    SECTION("invalid parameters")
    {
        REQUIRE_THROWS(nitro::rectangle { { -2, -124 }, { 0, 1 } });
        REQUIRE_THROWS(nitro::rectangle { { -2, -124 }, { 1, 0 } });
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
    REQUIRE(rect.parent() == nullptr);
    REQUIRE(rect.id() == 0);
}

TEST_CASE("full input parsing", "[io][basic]")
{
    const auto rect_json = R"-(
{
"rects": [
{"x": 100, "y": 300, "w": 250, "h": 80 },
{"x": 120, "y": 200, "w": 260, "h": 150 }
]
}
)-"_json;
    const auto rects     = nitro::to_rectangles(rect_json);
    REQUIRE(rects.size() == 2);
    REQUIRE(rects[0].origin().x == 100);
    REQUIRE(rects[0].origin().y == 300);
    REQUIRE(rects[0].width() == 250);
    REQUIRE(rects[0].height() == 80);
    REQUIRE(rects[0].id() == 1);
    REQUIRE(rects[0].parent() == nullptr);

    REQUIRE(rects[1].origin().x == 120);
    REQUIRE(rects[1].origin().y == 200);
    REQUIRE(rects[1].width() == 260);
    REQUIRE(rects[1].height() == 150);
    REQUIRE(rects[1].id() == 2);
    REQUIRE(rects[1].parent() == nullptr);
}

TEST_CASE("full input parsing invalid input", "[io][basic]")
{
    const auto rect_json = R"-({})-"_json;
    REQUIRE_THROWS(nitro::to_rectangles(rect_json));
}

TEST_CASE("rectangle ordering", "[basic][rectangle]")
{
    using nr = nitro::rectangle;

    nr r1 { { 100, 200 }, { 10, 20 }, id(1) };
    nr r2 { { 102, 201 }, { 10, 20 }, id(2) };
    nr r3 { { 102, 200 }, { 10, 20 }, id(3) };
    nr r4 { { 100, 200 }, { 10, 20 }, id(4) };
    nr r101 { { 101, 200 }, { 10, 20 }, id(101) };

    nitro::horizontal_sort h;
    SECTION("Sorting")
    {
        std::vector v { r1, r2, r1, r3, r4 };
        rng::sort(v, h);
        REQUIRE(v[0].id() == 1);
        REQUIRE(v[1].id() == 1);
        REQUIRE(v[2].id() == 4);
        REQUIRE(v[3].id() == 3);
        REQUIRE(v[4].id() == 2);
    }
    SECTION("Sorting of ptrs")
    {
        std::vector v { &r1, &r2, &r1, &r3, &r4 };
        rng::sort(v, h);
        REQUIRE(v[0]->id() == 1);
        REQUIRE(v[1]->id() == 1);
        REQUIRE(v[2]->id() == 4);
        REQUIRE(v[3]->id() == 3);
        REQUIRE(v[4]->id() == 2);
    }
    SECTION("compare with value")
    {
        REQUIRE(h(r1, 101));
        REQUIRE_FALSE(h(101, r1));
        REQUIRE(h(r4, 101));
        REQUIRE_FALSE(h(101, r4));
        REQUIRE_FALSE(h(r3, 101));
        REQUIRE(h(101, r3));
        REQUIRE_FALSE(h(r2, 101));
        REQUIRE(h(101, r2));
        REQUIRE_FALSE(h(101, r101));
        REQUIRE_FALSE(h(r101, 101));
    }
    SECTION("compare ptr with value")
    {
        REQUIRE(h(&r1, 101));
        REQUIRE_FALSE(h(101, &r1));
        REQUIRE(h(&r4, 101));
        REQUIRE_FALSE(h(101, &r4));
        REQUIRE_FALSE(h(&r3, 101));
        REQUIRE(h(101, &r3));
        REQUIRE_FALSE(h(&r2, 101));
        REQUIRE(h(101, &r2));
        REQUIRE_FALSE(h(101, &r101));
        REQUIRE_FALSE(h(&r101, 101));
    }
}

TEST_CASE("rectangle set", "[basic][rectangle]")
{
    using nr = nitro::rectangle;

    nr r1 { { 100, 200 }, { 10, 20 }, id(1) };
    nr r2 { { 102, 201 }, { 10, 20 }, id(2) };
    nr r3 { { 102, 200 }, { 10, 20 }, id(3) };
    nr r4 { { 100, 200 }, { 10, 20 }, id(4) };
    nr r101 { { 101, 200 }, { 10, 20 }, id(101) };
    nr r101_copy { { 101, 200 }, { 10, 20 }, id(101) };
    nr r_notinset { { 101, 200 }, { 10, 20 }, id(102) };

    std::vector v { &r1, &r2, &r3, &r4, &r101, &r2 };

    auto s = nitro::partition_tree::sorted<nitro::horizontal_sort>(v);
    REQUIRE(s.size() == 5);

    std::vector<nitro::rect_ptr> lower(s.begin(), s.lower_bound(101));
    std::vector<nitro::rect_ptr> at(s.lower_bound(101), s.upper_bound(101));
    std::vector<nitro::rect_ptr> upper(s.upper_bound(101), s.end());
    REQUIRE(lower.size() == 2);
    REQUIRE(lower[0]->id() == 1);
    REQUIRE(lower[1]->id() == 4);
    REQUIRE(at.size() == 1);
    REQUIRE(at[0]->id() == 101);
    REQUIRE(upper.size() == 2);
    REQUIRE(upper[0]->id() == 3);
    REQUIRE(upper[1]->id() == 2);
    REQUIRE(s.find(&r_notinset) == s.end());
    REQUIRE((*s.find(&r101_copy))->id() == 101);
}

#include <algorithm>
#include <catch2/catch.hpp>

#include "nitro/fwd.hpp"
#include "nitro/memory_resource.hpp"
#include "nitro/rectangle.hpp"
#include "nitro/utils.hpp"
#include "test_utils.hpp"

#include <compare>
#include <iostream>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <nitro/io.hpp>
#include <nitro/partition_tree.hpp>

#include <nlohmann/json.hpp>
#include <ranges>

using namespace nitro;

TEST_CASE("space construction", "[partition_tree]")
{
    const auto input = R"-(
        {
"rects": [
{"x": 100, "y": 100, "w": 250, "h": 80 },
{"x": 120, "y": 200, "w": 250, "h": 150 },
{"x": 140, "y": 160, "w": 250, "h": 100 },
{"x": 160, "y": 140, "w": 350, "h": 190 }
]
}
    )-"_json;

    auto                  rects = (to_rectangles(input));
    std::vector<rect_ptr> rect_view;
    rng::copy(rects | views::transform(address_of_f {}), std::back_inserter(rect_view));

    SECTION("horizontal ordering")
    {
        auto h  = sorted<vertical>(rect_view);
        auto it = h.begin();
        REQUIRE(it != h.end());
        REQUIRE((*it++)->id() == 1);
        REQUIRE((*it++)->id() == 2);
        REQUIRE((*it++)->id() == 3);
        REQUIRE((*it++)->id() == 4);
        REQUIRE(it == h.end());
    }
    SECTION("vertical ordering")
    {
        auto v  = sorted<horizontal>(rect_view);
        auto it = v.begin();
        REQUIRE(it != v.end());
        REQUIRE((*it++)->id() == 1);
        REQUIRE((*it++)->id() == 4);
        REQUIRE((*it++)->id() == 3);
        REQUIRE((*it++)->id() == 2);
        REQUIRE(it == v.end());
    }
}

TEST_CASE("rectangle slicing horizontal", "[rectangle][geometry]")
{
    rectangle r1 { { 100, 200 }, { 250, 80 }, id(241) };
    SECTION("out of bounds from below")
    {
        const auto [below, above] = r1.slice(horizontal { 199 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("on edge from below")
    {
        const auto [below, above] = r1.slice(horizontal { 200 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("fist in between")
    {
        const auto& [below, above] = r1.slice(horizontal { 201 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 250, .y = 1 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 100, .y = 201 });
        REQUIRE(above->extent() == point { .x = 250, .y = 79 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("in between")
    {
        const auto& [below, above] = r1.slice(horizontal { 237 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 250, .y = 37 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 100, .y = 237 });
        REQUIRE(above->extent() == point { .x = 250, .y = 43 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("last in between")
    {
        const auto& [below, above] = r1.slice(horizontal { 279 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 250, .y = 79 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 100, .y = 279 });
        REQUIRE(above->extent() == point { .x = 250, .y = 1 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("on edge from above")
    {
        const auto [below, above] = r1.slice(horizontal { 280 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
    SECTION("out of bounds from  above")
    {
        const auto [below, above] = r1.slice(horizontal { 280 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
}

TEST_CASE("rectangle slicing rev vertical", "[rectangle][geometry]")
{
    rectangle r1 { { 160, 200 }, { 210, 150 }, id(2) };
    SECTION("out of bounds from below")
    {
        const auto [below, above] = r1.slice(rev_vertical { 370 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("fist in between")
    {
        const auto& [below, above] = r1.slice(rev_vertical { 369 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 369, .y = 200 });
        REQUIRE(below->extent() == point { .x = 1, .y = 150 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 2);
        REQUIRE(above->origin() == point { .x = 160, .y = 200 });
        REQUIRE(above->extent() == point { .x = 209, .y = 150 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 2);
    }
    SECTION("last in between")
    {
        const auto& [below, above] = r1.slice(rev_vertical { 161 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 161, .y = 200 });
        REQUIRE(below->extent() == point { .x = 209, .y = 150 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 2);
        REQUIRE(above->origin() == point { .x = 160, .y = 200 });
        REQUIRE(above->extent() == point { .x = 1, .y = 150 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 2);
    }
    SECTION("on edge from above")
    {
        const auto [below, above] = r1.slice(rev_vertical { 160 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
}
TEST_CASE("rectangle slicing rev horizontal", "[rectangle][geometry]")
{
    rectangle r1 { { 160, 200 }, { 210, 150 }, id(2) };
    SECTION("out of bounds from below")
    {
        const auto [below, above] = r1.slice(rev_horizontal { 350 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("fist in between")
    {
        const auto& [below, above] = r1.slice(rev_horizontal { 349 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 160, .y = 349 });
        REQUIRE(below->extent() == point { .x = 210, .y = 1 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 2);
        REQUIRE(above->origin() == point { .x = 160, .y = 200 });
        REQUIRE(above->extent() == point { .x = 210, .y = 149 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 2);
    }
    SECTION("last in between")
    {
        const auto& [below, above] = r1.slice(rev_horizontal { 201 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 160, .y = 201 });
        REQUIRE(below->extent() == point { .x = 210, .y = 149 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 2);
        REQUIRE(above->origin() == point { .x = 160, .y = 200 });
        REQUIRE(above->extent() == point { .x = 210, .y = 1 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 2);
    }
    SECTION("on edge from above")
    {
        const auto [below, above] = r1.slice(rev_horizontal { 200 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
}
TEST_CASE("rectangle slicing vertical", "[rectangle][geometry]")
{
    rectangle r1 { { 100, 200 }, { 250, 80 }, id(241) };
    SECTION("out of bounds from below")
    {
        const auto [below, above] = r1.slice(vertical { 99 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("on edge from below")
    {
        const auto [below, above] = r1.slice(vertical { 100 });
        REQUIRE(!below);
        REQUIRE(std::is_eq(*above <=> r1));
    }
    SECTION("fist in between")
    {
        const auto& [below, above] = r1.slice(vertical { 101 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 1, .y = 80 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 101, .y = 200 });
        REQUIRE(above->extent() == point { .x = 249, .y = 80 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("in between")
    {
        const auto& [below, above] = r1.slice(vertical { 151 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 51, .y = 80 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 151, .y = 200 });
        REQUIRE(above->extent() == point { .x = 199, .y = 80 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("last in between")
    {
        const auto& [below, above] = r1.slice(vertical { 349 });
        REQUIRE(std::is_neq(*below <=> r1));
        REQUIRE(std::is_neq(*above <=> r1));
        REQUIRE(below->origin() == point { .x = 100, .y = 200 });
        REQUIRE(below->extent() == point { .x = 249, .y = 80 });
        REQUIRE(below->parent() == &r1);
        REQUIRE(below->id() == 241);
        REQUIRE(above->origin() == point { .x = 349, .y = 200 });
        REQUIRE(above->extent() == point { .x = 1, .y = 80 });
        REQUIRE(above->parent() == &r1);
        REQUIRE(above->id() == 241);
    }
    SECTION("on edge from above")
    {
        const auto [below, above] = r1.slice(vertical { 350 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
    SECTION("out of bounds from  above")
    {
        const auto [below, above] = r1.slice(vertical { 351 });
        REQUIRE(std::is_eq(*below <=> r1));
        REQUIRE(!above);
    }
}

auto test_data()
{
    using nr = nitro::rectangle;
    nitro::rectangles_list rects { nr { { 100, 100 }, { 250, 80 }, id(1) },
        nr { { 120, 200 }, { 250, 150 }, id(2) }, nr { { 140, 160 }, { 250, 100 }, id(3) },
        nr { { 160, 140 }, { 350, 190 }, id(4) } };
    return rects;
}
template <typename Ord> auto sorted_test_data(Ord ord)
{
    auto              td = test_data();
    sorted_rectangles sr(ordering_t { ord });
    rng::copy(td | views::transform([](auto& r) { return std::addressof(r); }),
        std::inserter(sr, sr.begin()));
    std::vector<rect_ptr> vtd;
    rng::copy(td | views::transform(address_of_f {}), std::back_inserter(vtd));
    return std::tuple { std::move(td), std::move(vtd), std::move(sr) };
}
TEST_CASE("space slicing horizontal", "[partition_tree]")
{
    auto&& [td, vtd, sorted_td]  = sorted_test_data(vertical_sort {});
    auto&& [rects, below, above] = partition_tree::slice(horizontal { 190 }, sorted_td);
    REQUIRE(below.size() == 3);
    REQUIRE(above.size() == 3);
    auto it = below.begin();
    REQUIRE((*it)->id() == 1);
    REQUIRE((*it)->origin() == point { 100, 100 });
    REQUIRE((*it)->extent() == point { 250, 80 });
    REQUIRE((*it)->parent() == nullptr);
    ++it;
    REQUIRE((*it)->id() == 4);
    REQUIRE((*it)->origin() == point { 160, 140 });
    REQUIRE((*it)->extent() == point { 350, 50 });
    REQUIRE((*it)->parent() == vtd[3]);
    ++it;
    REQUIRE((*it)->id() == 3);
    REQUIRE((*it)->origin() == point { 140, 160 });
    REQUIRE((*it)->extent() == point { 250, 30 });
    REQUIRE((*it)->parent() == vtd[2]);

    ++it;
    REQUIRE(it == below.end());

    auto ait = above.begin();
    REQUIRE((*ait)->id() == 3);
    REQUIRE((*ait)->origin() == point { 140, 190 });
    REQUIRE((*ait)->extent() == point { 250, 70 });
    REQUIRE((*ait)->parent() == vtd[2]);

    ++ait;
    REQUIRE((*ait)->id() == 4);
    REQUIRE((*ait)->origin() == point { 160, 190 });
    REQUIRE((*ait)->extent() == point { 350, 140 });
    REQUIRE((*ait)->parent() == vtd[3]);
    ++ait;
    REQUIRE((*ait)->id() == 2);
    REQUIRE((*ait)->origin() == point { 120, 200 });
    REQUIRE((*ait)->extent() == point { 250, 150 });
    REQUIRE((*ait)->parent() == nullptr);
    ++ait;
    REQUIRE(ait == above.end());
}

TEST_CASE("space slicing vertical", "[partition_tree]")
{
    auto&& [td, vtd, sorted_td]  = sorted_test_data(horizontal_sort {});
    auto&& [rects, below, above] = partition_tree::slice(vertical { 140 }, sorted_td);
    REQUIRE(below.size() == 2);
    REQUIRE(above.size() == 4);
    auto it = below.begin();
    REQUIRE((*it)->id() == 1);
    REQUIRE((*it)->origin() == point { 100, 100 });
    REQUIRE((*it)->extent() == point { 40, 80 });
    REQUIRE((*it)->parent() == vtd[0]);
    ++it;
    REQUIRE((*it)->id() == 2);
    REQUIRE((*it)->origin() == point { 120, 200 });
    REQUIRE((*it)->extent() == point { 20, 150 });
    REQUIRE((*it)->parent() == vtd[1]);
    ++it;
    REQUIRE(it == below.end());

    auto ait = above.begin();
    REQUIRE((*ait)->id() == 1);
    REQUIRE((*ait)->origin() == point { 140, 100 });
    REQUIRE((*ait)->extent() == point { 210, 80 });
    REQUIRE((*ait)->parent() == vtd[0]);
    ++ait;
    REQUIRE((*ait)->id() == 3);
    REQUIRE((*ait)->origin() == point { 140, 160 });
    REQUIRE((*ait)->extent() == point { 250, 100 });
    REQUIRE((*ait)->parent() == nullptr);
    ++ait;
    REQUIRE((*ait)->id() == 2);
    REQUIRE((*ait)->origin() == point { 140, 200 });
    REQUIRE((*ait)->extent() == point { 230, 150 });
    REQUIRE((*ait)->parent() == vtd[1]);
    ++ait;
    REQUIRE((*ait)->id() == 4);
    REQUIRE((*ait)->origin() == point { 160, 140 });
    REQUIRE((*ait)->extent() == point { 350, 190 });
    REQUIRE((*ait)->parent() == nullptr);
    ++ait;
    REQUIRE(ait == above.end());
}

TEST_CASE("space partitioning example", "[partition_tree]")
{
    auto&& [td, vtd, sorted_td] = sorted_test_data(horizontal_sort {});
    partition_tree pt(std::move(td));
    auto           intersections = pt.intersections();
    std::vector    vi(intersections.begin(), intersections.end());
    REQUIRE(intersections.size() == 7);
    SECTION("1st intersection")
    {
        auto&      is = vi[0];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 2);
        REQUIRE(is.constituents().at(0)->id() == 1);
        REQUIRE(is.constituents().at(1)->id() == 3);
        REQUIRE(i.origin() == point { 140, 160 });
        REQUIRE(i.extent() == point { 210, 20 });
    }
    SECTION("2nd intersection")
    {
        auto&      is = vi[1];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 2);
        REQUIRE(is.constituents().at(0)->id() == 1);
        REQUIRE(is.constituents().at(1)->id() == 4);
        REQUIRE(i.origin() == point { 160, 140 });
        REQUIRE(i.extent() == point { 190, 40 });
    }
    SECTION("3rd intersection")
    {
        auto&      is = vi[2];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 2);
        REQUIRE(is.constituents().at(0)->id() == 2);
        REQUIRE(is.constituents().at(1)->id() == 3);
        REQUIRE(i.origin() == point { 140, 200 });
        REQUIRE(i.extent() == point { 230, 60 });
    }
    SECTION("4th intersection")
    {
        auto&      is = vi[3];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 2);
        REQUIRE(is.constituents().at(0)->id() == 2);
        REQUIRE(is.constituents().at(1)->id() == 4);
        REQUIRE(i.origin() == point { 160, 200 });
        REQUIRE(i.extent() == point { 210, 130 });
    }
    SECTION("5th intersection")
    {
        auto&      is = vi[4];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 2);
        REQUIRE(is.constituents().at(0)->id() == 3);
        REQUIRE(is.constituents().at(1)->id() == 4);
        REQUIRE(i.origin() == point { 160, 160 });
        REQUIRE(i.extent() == point { 230, 100 });
    }
    SECTION("6th intersection")
    {
        auto&      is = vi[5];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 3);
        REQUIRE(is.constituents().at(0)->id() == 1);
        REQUIRE(is.constituents().at(1)->id() == 3);
        REQUIRE(is.constituents().at(2)->id() == 4);
        REQUIRE(i.origin() == point { 160, 160 });
        REQUIRE(i.extent() == point { 190, 20 });
    }
    SECTION("7th intersection")
    {
        auto&      is = vi[6];
        const auto i  = is.calculate();
        REQUIRE(is.constituents().size() == 3);
        REQUIRE(is.constituents().at(0)->id() == 2);
        REQUIRE(is.constituents().at(1)->id() == 3);
        REQUIRE(is.constituents().at(2)->id() == 4);
        REQUIRE(i.origin() == point { 160, 200 });
        REQUIRE(i.extent() == point { 210, 60 });
    }
}

auto get_concentric_rectangles(coordinate_t count)
{

    nitro::rectangles_list rects;
    std::generate_n(
        std::front_inserter(rects), count, [id = 1, x = 0, y = 2 * count - 1]() mutable {
            auto r = rectangle { { x, x }, { y, y },
                rectangle::identifier_t(static_cast<size_t>(id++)) };
            x += 1;
            y -= 2;
            return r;
        });
    return rects;
}

TEST_CASE("space partitioning   concentrical rectangles", "[partition_tree]")
{
    auto old_resource = std::pmr::get_default_resource();
    auto pool         = get_default_memory_resource(old_resource);
    std::pmr::set_default_resource(&pool);

    SECTION("3 rects")
    {
        auto           l = get_concentric_rectangles(3);
        partition_tree pt(std::move(l));
        auto           i = pt.intersections();
        REQUIRE(i.size() == 2);
        std::vector v(i.begin(), i.end());
        REQUIRE(v[0].calculate().origin() == point { 1, 1 });
        REQUIRE(v[0].calculate().extent() == point { 3, 3 });
        REQUIRE(v[0].constituents().size() == 2);
        REQUIRE(v[1].calculate().origin() == point { 2, 2 });
        REQUIRE(v[1].calculate().extent() == point { 1, 1 });
        REQUIRE(v[1].constituents().size() == 3);
    }
    SECTION("10 rects")
    {
        auto           l = get_concentric_rectangles(10);
        partition_tree pt(std::move(l));
        REQUIRE(pt.intersections().size() == 9);
    }
    SECTION("100 rects")
    {
        auto           l = get_concentric_rectangles(100);
        partition_tree pt(std::move(l));
        REQUIRE(pt.intersections().size() == 99);
    }
    SECTION("100 rects same extent")
    {
        rectangles_list rects;
        std::generate_n(std::back_inserter(rects), 100, [idd = 1]() mutable {
            return rectangle { { 100, 100 }, { 100, 100 }, id(idd++) };
        });
        partition_tree pt(std::move(rects));
        REQUIRE(pt.intersections().size() == 1);
        REQUIRE(pt.intersections().begin()->constituents().size() == 100);
        REQUIRE(pt.intersections().begin()->calculate().origin() == point{100,100});
        REQUIRE(pt.intersections().begin()->calculate().extent() == point{100,100});
    }
    std::pmr::set_default_resource(old_resource);
}

TEST_CASE("rectangle set aligned", "[basic][rectangle]")
{
    using nr = nitro::rectangle;

    nr r1 { { 160, 200 }, { 210, 150 }, id(1) };
    nr r2 { { 160, 200 }, { 210, 60 }, id(2) };
    nr r3 { { 160, 200 }, { 210, 130 }, id(3) };

    using pt = nitro::partition_tree;
    std::vector v { &r1, &r2, &r3 };
    SECTION("horizontal")
    {
        using o      = nitro::horizontal;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 200);
        auto [rects, below, above] = pt::slice(o { p }, s);

        auto rv_below = rect_vec(below);
        auto rv_above = rect_vec(above);
        REQUIRE(rv_below.empty());
        REQUIRE(rv_above.size() == 3);
        REQUIRE(std::is_eq(rv_above[0] <=> r2));
        REQUIRE(std::is_eq(rv_above[1] <=> r3));
        REQUIRE(std::is_eq(rv_above[2] <=> r1));
    }
    SECTION("vertical")
    {
        using o      = nitro::vertical;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 160);
        auto [rects, below, above] = pt::slice(o { p }, s);

        auto rv_below = rect_vec(below);
        auto rv_above = rect_vec(above);
        REQUIRE(rv_below.empty());
        REQUIRE(rv_above.size() == 3);
        REQUIRE(std::is_eq(rv_above[0] <=> r2));
        REQUIRE(std::is_eq(rv_above[1] <=> r3));
        REQUIRE(std::is_eq(rv_above[2] <=> r1));
    }
    SECTION("rev horizontal")
    {
        using o = nitro::rev_horizontal;
        auto s  = sorted<o>(v);
        auto sp = rect_vec(s);

        REQUIRE(std::is_eq(sp[0] <=> r1));
        REQUIRE(std::is_eq(sp[1] <=> r3));
        REQUIRE(std::is_eq(sp[2] <=> r2));
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == (r2.origin().y + r2.height()));
        auto [rects, below, above] = pt::slice(o { p }, s);

        auto rv_below = rect_vec(below);
        auto rv_above = rect_vec(above);
        REQUIRE(rv_below.size() == 2);
        REQUIRE(rv_above.size() == 3);
        REQUIRE(rv_above[0].id() == r3.id());
        REQUIRE(rv_above[1].id() == r2.id());
        REQUIRE(rv_above[2].id() == r1.id());
        REQUIRE((rv_above[0].origin() == rv_above[1].origin()
            && rv_above[1].origin() == rv_above[2].origin()));
        REQUIRE((rv_above[0].extent() == rv_above[1].extent()
            && rv_above[1].extent() == rv_above[2].extent()));
        REQUIRE(rv_below[0].id() == 1);
        REQUIRE(rv_below[1].id() == 3);
        REQUIRE((rv_below[0].origin() == rv_below[1].origin()));
        REQUIRE((rv_below[1].extent() == nitro::point { 210, 70 }));
        REQUIRE((rv_below[0].extent() == nitro::point { 210, 90 }));
    }
    SECTION("rev vertical")
    {
        using o      = nitro::rev_vertical;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 370);
        auto [rects, below, above] = pt::slice(o { p }, s);
        REQUIRE(below.empty());
        auto rv_above = rect_vec(above);
        REQUIRE(rv_above.size() == 3);
        REQUIRE(std::is_eq(rv_above[0] <=> r1));
        REQUIRE(std::is_eq(rv_above[1] <=> r3));
        REQUIRE(std::is_eq(rv_above[2] <=> r2));
    }
}

TEST_CASE("rectangle set aligned rotated", "[basic][rectangle]")
{
    using nr = nitro::rectangle;

    nr r1 { { 160, 200 }, { 150, 210 }, id(1) };
    nr r2 { { 160, 200 }, { 60, 210 }, id(2) };
    nr r3 { { 160, 200 }, { 130, 210 }, id(3) };

    using pt = nitro::partition_tree;
    std::vector v { &r1, &r2, &r3 };
    SECTION("vertical")
    {
        using o      = nitro::vertical;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 160);
        auto [rects, below, above] = pt::slice(o { p }, s);
        REQUIRE(below.empty());
        auto rv_above = rect_vec(above);
        REQUIRE(rv_above.size() == 3);
        REQUIRE(std::is_eq(rv_above[0] <=> r2));
        REQUIRE(std::is_eq(rv_above[1] <=> r3));
        REQUIRE(std::is_eq(rv_above[2] <=> r1));
    }
    SECTION("rev vertical")
    {
        using o      = nitro::rev_vertical;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 220);
        auto [rects, below, above] = pt::slice(o { p }, s);
        auto rv_below              = rect_vec(below);
        auto rv_above              = rect_vec(above);
        REQUIRE(rv_below.size() == 2);
        REQUIRE(rv_above.size() == 3);
        REQUIRE(rv_above[0].id() == r3.id());
        REQUIRE(rv_above[1].id() == r2.id());
        REQUIRE(rv_above[2].id() == r1.id());
        REQUIRE((rv_above[0].origin() == rv_above[1].origin()
            && rv_above[1].origin() == rv_above[2].origin()));
        REQUIRE((rv_above[0].extent() == rv_above[1].extent()
            && rv_above[1].extent() == rv_above[2].extent()));
        REQUIRE(rv_below[0].id() == 1);
        REQUIRE(rv_below[1].id() == 3);
        REQUIRE((rv_below[0].origin() == rv_below[1].origin()));
        REQUIRE((rv_below[1].extent() == nitro::point { 70, 210 }));
        REQUIRE((rv_below[0].extent() == nitro::point { 90, 210 }));
    }
}

TEST_CASE("intersection creation", "[partition_tree][intersection]")
{
    using ints = nitro::partition_tree::intersection;

    using nr = nitro::rectangle;

    nr r1 { { 160, 200 }, { 210, 150 }, id(1) };
    nr r2 { { 160, 200 }, { 210, 60 }, id(2) };
    nr r3 { { 160, 200 }, { 210, 130 }, id(3) };

    using pt = nitro::partition_tree;
    SECTION("intersection is an ordered set")
    {
        std::vector is { &r2, &r3, &r3, &r1, &r1 };
        ints        i1(is);
        REQUIRE(i1.constituents().size() == 3);
        REQUIRE(i1.constituents()[0]->id() == 1);
        REQUIRE(i1.constituents()[1]->id() == 2);
        REQUIRE(i1.constituents()[2]->id() == 3);
        SECTION("intersections compare lexicographically")
        {
            SECTION("equivalent")
            {
                std::vector is_eq { &r3, &r2, &r3, &r3, &r1, &r1 };
                ints        i2(is_eq);
                REQUIRE_FALSE(i2 < i1);
                REQUIRE_FALSE(i1 < i2);
            }
            SECTION("less or greater")
            {
                std::vector less { &r3, &r2 };
                ints        i2(less);
                REQUIRE(i2 < i1);
                REQUIRE_FALSE(i1 < i2);
            }
        }
    }
}

TEST_CASE("error from 1000 concentric", "[partition_tree][edge_case]")
{
    using nr = nitro::rectangle;

    nr r1 { { 3, 2 }, { 1, 3 }, id(1) };
    nr r2 { { 3, 3 }, { 1, 2 }, id(2) };

    std::vector v { &r1, &r2 };
    using pt = partition_tree;
    SECTION("horizontal")
    {
        using o      = nitro::horizontal;
        auto       s = sorted<o>(v);
        const auto p = pt::split_point<o>(s);
        REQUIRE(p == 3);
        auto [rects, below, above] = pt::slice(o { p }, s);
        REQUIRE(below.size() == 1);
        REQUIRE(above.size() == 2);
        auto rv_below = rect_vec(below);
        auto rv_above = rect_vec(above);
        REQUIRE(rv_below[0].origin() == point { 3, 2 });
        REQUIRE(rv_below[0].extent() == point { 1, 1 });
        REQUIRE(rv_below[0].id() == 1);
        REQUIRE(rv_above[0].origin() == point { 3, 3 });
        REQUIRE(rv_above[0].extent() == point { 1, 2 });
        REQUIRE(rv_above[0].id() == 1);
        REQUIRE(rv_above[1].origin() == point { 3, 3 });
        REQUIRE(rv_above[1].extent() == point { 1, 2 });
        REQUIRE(rv_above[1].id() == 2);
    }
    SECTION("partition tree")
    {
        partition_tree p(rectangles_list { r1, r2 });
        REQUIRE(!p.intersections().empty());
    }
}

// TODO: intesection list to printout

// TODO: add coverage
// TODO: add readme
// TODO: add binary app (withs support of timeout)

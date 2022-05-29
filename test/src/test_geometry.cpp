#include <catch2/catch.hpp>

#include "nitro/utils.hpp"
#include "test_utils.hpp"

#include <compare>
#include <memory>
#include <nitro/geometry.hpp>
#include <nitro/io.hpp>

#include <nlohmann/json.hpp>
#include <ranges>

using namespace nitro;

TEST_CASE("space construction", "[space]")
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
        auto h  = partition_tree::sorted<horizontal_sort>(rect_view);
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
        auto v  = partition_tree::sorted<vertical_sort>(rect_view);
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
    std::list rects { nr { { 100, 100 }, { 250, 80 }, id(1) },
        nr { { 120, 200 }, { 250, 150 }, id(2) }, nr { { 140, 160 }, { 250, 100 }, id(3) },
        nr { { 160, 140 }, { 350, 190 }, id(4) } };
    return rects;
}
template <typename Ord> auto sorted_test_data(Ord ord)
{
    auto                              td = test_data();
    partition_tree::sorted_rectangles sr(ordering_t { ord });
    rng::copy(td | views::transform([](auto& r) { return std::addressof(r); }),
        std::inserter(sr, sr.begin()));
    std::vector<rect_ptr> vtd;
    rng::copy(td | views::transform(address_of_f {}), std::back_inserter(vtd));
    return std::tuple { std::move(td), std::move(vtd), std::move(sr) };
}
TEST_CASE("space slicing horizontal", "[space]")
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

TEST_CASE("space slicing vertical", "[space]")
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

TEST_CASE("space partitioning vertical", "[space]")
{
    auto&& [td, vtd, sorted_td] = sorted_test_data(horizontal_sort {});
    partition_tree pt(std::move(td));
}
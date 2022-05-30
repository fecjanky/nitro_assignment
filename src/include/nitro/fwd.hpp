#pragma once
#include <tuple>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <nitro/exceptions.hpp>
#include <ranges>
#include <type_traits>
#include <memory_resource>

namespace rng   = std::ranges;
namespace views = std::views;
namespace nitro {
using coordinate_t = int64_t;

struct point;
struct rectangle;

struct vertical;
struct rev_vertical;
struct horizontal;
struct rev_horizontal;
struct partition_tree;

using rect_ptr = gsl::not_null<rectangle const*>;

template <typename T>
concept RectPtrRange = rng::forward_range<T> && requires(T& t)
{
    {
        std::addressof(**t.begin())
        } -> std::convertible_to<rect_ptr>;
};

}
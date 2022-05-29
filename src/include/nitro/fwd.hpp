#pragma once
#include <nitro/exceptions.hpp>
#include <ranges>

namespace rng   = std::ranges;
namespace views = std::views;
namespace nitro {
using coordinate_t = int64_t;

struct point;
struct rectangle;
}
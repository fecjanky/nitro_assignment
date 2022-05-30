#pragma once
#include <nitro/fwd.hpp>

namespace nitro {

struct point {
    coordinate_t x {}, y {};
    auto         operator<=>(const point&) const = default;
};

struct orientation {
    coordinate_t val {};
};
}
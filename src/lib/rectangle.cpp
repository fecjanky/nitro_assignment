#include "nitro/fwd.hpp"
#include <nitro/rectangle.hpp>
#include <nitro/sorting_and_orientation.hpp>
#include <optional>

namespace nitro {
template <typename SlicePolicy>
rectangle::slice_t slice_impl(SlicePolicy const& policy, rectangle const& r)
{
    const auto ref = policy.ref(r);

    // internal slice
    if (policy.inner_slice(r)) {
        return policy.slice(r);
    }
    // on above slices
    if (policy.above(ref)) {
        return { std::nullopt, r };
    }
    // below
    return { r, std::nullopt };
}
auto rectangle::slice(vertical const& v) const -> slice_t { return slice_impl(v, *this); }
auto rectangle::slice(horizontal const& h) const -> slice_t { return slice_impl(h, *this); }
auto rectangle::slice(rev_vertical const& rv) const -> slice_t { return slice_impl(rv, *this); }
auto rectangle::slice(rev_horizontal const& rh) const -> slice_t { return slice_impl(rh, *this); }

std::optional<rectangle> rectangle::intersect(
    rectangle const& lhs, std::optional<rectangle> const& rhs_opt)

{
    if (rhs_opt == std::nullopt)
        return std::nullopt;
    auto&                 rhs = *rhs_opt;
    constexpr static auto _x  = [](auto&& v) { return v.x; };
    constexpr static auto _y  = [](auto&& v) { return v.y; };
    auto                  get_extremes
        = [](auto&& by, auto& l, auto& r) -> std::optional<std::pair<coordinate_t, coordinate_t>> {
        auto&& [nearest, other] = by(l.origin()) < by(r.origin()) ? std::tie(l, r) : std::tie(r, l);
        const auto furthest_of_nearest_p = by(nearest.origin()) + by(nearest.extent());
        const auto furthest_of_other_p   = by(other.origin()) + by(other.extent());
        const auto other_origin          = by(other.origin());
        if (furthest_of_nearest_p <= other_origin)
            return std::nullopt;
        return std::pair { other_origin, std::min(furthest_of_nearest_p, furthest_of_other_p) };
    };
    auto x_extremes = get_extremes(_x, lhs, rhs);
    if (!x_extremes) {
        return std::nullopt;
    }
    auto y_extremes = get_extremes(_y, lhs, rhs);
    if (!y_extremes) {
        return std::nullopt;
    }
    auto [origin_x, extent_x] = *x_extremes;
    auto [origin_y, extent_y] = *y_extremes;

    return rectangle { { origin_x, origin_y }, { extent_x - origin_x, extent_y - origin_y } };
}

}
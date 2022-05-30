#include "nitro/fwd.hpp"
#include <nitro/rectangle.hpp>
#include <nitro/sorting_and_orientation.hpp>
#include <numeric>

namespace nitro {

std::strong_ordering horizontal_sort::compare(
    const rectangle& lhs, const rectangle& rhs) const noexcept
{
    const auto lho = lhs.origin();
    const auto rho = rhs.origin();
    const auto lw  = lhs.width();
    const auto lh  = lhs.height();
    const auto lid = lhs.id();
    const auto rw  = rhs.width();
    const auto rh  = rhs.height();
    const auto rid = rhs.id();
    return std::tie(lho.x, lho.y, lw, lh, lid) <=> std::tie(rho.x, rho.y, rw, rh, rid);
}
std::weak_ordering horizontal_sort::compare(const rectangle& lhs, coordinate_t rhs) const noexcept
{
    return lhs.origin().x <=> rhs;
}
std::weak_ordering horizontal_sort::compare(coordinate_t lhs, rectangle const& rhs) const noexcept
{
    return lhs <=> rhs.origin().x;
}
// coordinate_t horizontal_sort::reference(rectangle const& r) const { return r.origin().x; }

std::strong_ordering rev_horizontal_sort::compare(
    const rectangle& lhs, const rectangle& rhs) const noexcept
{
    const auto lho_x = lhs.origin().x + lhs.width();
    const auto lho_y = lhs.origin().y + lhs.height();
    const auto rho_x = rhs.origin().x + rhs.width();
    const auto rho_y = rhs.origin().y + rhs.height();
    const auto lw    = lhs.width();
    const auto lh    = lhs.height();
    const auto lid   = lhs.id();
    const auto rw    = rhs.width();
    const auto rh    = rhs.height();
    const auto rid   = rhs.id();
    return std::tie(rho_x, rho_y, rw, rh, rid) <=> std::tie(lho_x, lho_y, lw, lh, lid);
}
std::weak_ordering rev_horizontal_sort::compare(
    const rectangle& lhs, coordinate_t rhs) const noexcept
{
    return rhs <=> (lhs.origin().x + lhs.width());
}
std::weak_ordering rev_horizontal_sort::compare(
    coordinate_t lhs, rectangle const& rhs) const noexcept
{
    return (rhs.origin().x + rhs.width()) <=> lhs;
}

std::strong_ordering vertical_sort::compare(
    const rectangle& lhs, const rectangle& rhs) const noexcept
{
    const auto lho = lhs.origin();
    const auto rho = rhs.origin();
    const auto lw  = lhs.width();
    const auto lh  = lhs.height();
    const auto lid = lhs.id();
    const auto rw  = rhs.width();
    const auto rh  = rhs.height();
    const auto rid = rhs.id();
    return std::tie(lho.y, lho.x, lh, lw, lid) <=> std::tie(rho.y, rho.x, rh, rw, rid);
}
std::weak_ordering vertical_sort::compare(const rectangle& lhs, coordinate_t rhs) const noexcept
{
    return lhs.origin().y <=> rhs;
}
std::weak_ordering vertical_sort::compare(coordinate_t lhs, rectangle const& rhs) const noexcept
{
    return lhs <=> rhs.origin().y;
}

std::strong_ordering rev_vertical_sort::compare(
    const rectangle& lhs, const rectangle& rhs) const noexcept
{
    const auto lho_x = lhs.origin().y + lhs.height();
    const auto lho_y = lhs.origin().x + lhs.width();
    const auto rho_x = rhs.origin().y + lhs.height();
    const auto rho_y = rhs.origin().x + rhs.width();
    const auto lw    = lhs.width();
    const auto lh    = lhs.height();
    const auto lid   = lhs.id();
    const auto rw    = rhs.width();
    const auto rh    = rhs.height();
    const auto rid   = rhs.id();
    return std::tie(rho_x, rho_y, rh, rw, rid) <=> std::tie(lho_x, lho_y, lh, lw, lid);
}
std::weak_ordering rev_vertical_sort::compare(const rectangle& lhs, coordinate_t rhs) const noexcept
{
    return rhs <=> (lhs.origin().y + lhs.height());
}
std::weak_ordering rev_vertical_sort::compare(coordinate_t lhs, rectangle const& rhs) const noexcept
{
    return (rhs.origin().y + rhs.height()) <=> lhs;
}

coordinate_t rev_vertical::ref(const rectangle& r) const noexcept
{
    return r.origin().x + r.width();
}
bool rev_vertical::inner_slice(const rectangle& r) const noexcept
{
    return r.origin().x < val && val < r.origin().x + r.width();
}
bool rev_vertical::above(coordinate_t c) const noexcept { return c <= val; }

std::pair<rectangle, rectangle> rev_vertical::slice(const rectangle& r) const noexcept
{
    coordinate_t slice_width { ref(r) - val };
    assert(slice_width > 0);
    return {
        rectangle { { val, r.origin().y }, { slice_width, r.height() }, &r },
        rectangle { r.origin(), { r.width() - slice_width, r.height() }, &r },
    };
}

coordinate_t horizontal::ref(const rectangle& r) const noexcept { return r.origin().y; }
bool         horizontal::inner_slice(const rectangle& r) const noexcept
{
    return r.origin().y < val && val < r.origin().y + r.height();
}
bool horizontal::above(coordinate_t c) const noexcept { return c >= val; }

std::pair<rectangle, rectangle> horizontal::slice(const rectangle& r) const noexcept
{
    coordinate_t slice_width { val - r.origin().y };
    assert(slice_width > 0);
    return { rectangle { r.origin(), { r.width(), slice_width }, &r },
        rectangle { { r.origin().x, r.origin().y + slice_width },
            { r.width(), r.height() - slice_width }, &r } };
}

coordinate_t vertical::ref(const rectangle& r) const noexcept { return r.origin().x; }
bool         vertical::inner_slice(const rectangle& r) const noexcept
{
    return r.origin().x < val && val < r.origin().x + r.width();
}
bool vertical::above(coordinate_t c) const noexcept { return c >= val; }

std::pair<rectangle, rectangle> vertical::slice(const rectangle& r) const noexcept
{
    coordinate_t slice_width { val - r.origin().x };
    assert(slice_width > 0);
    return { rectangle { r.origin(), { slice_width, r.height() }, &r },
        rectangle { { r.origin().x + slice_width, r.origin().y },
            { r.width() - slice_width, r.height() }, &r } };
}

coordinate_t rev_horizontal::ref(const rectangle& r) const noexcept
{
    return r.origin().y + r.height();
}
bool rev_horizontal::inner_slice(const rectangle& r) const noexcept
{
    return r.origin().y < val && val < r.origin().y + r.height();
}
bool rev_horizontal::above(coordinate_t c) const noexcept { return c <= val; }

std::pair<rectangle, rectangle> rev_horizontal::slice(const rectangle& r) const noexcept
{
    coordinate_t slice_width { ref(r) - val };
    assert(slice_width > 0);
    return {
        rectangle { { r.origin().x, val }, { r.width(), slice_width }, &r },
        rectangle { r.origin(), { r.width(), r.height() - slice_width }, &r },
    };
}
namespace {
    auto round_up(coordinate_t a, coordinate_t b)
    {
        return std::midpoint(std::max(a, b), std::min(a, b));
    }
    auto round_down(coordinate_t a, coordinate_t b)
    {
        return std::midpoint(std::min(a, b), std::max(a, b));
    }
}
coordinate_t horizontal::midpoint(coordinate_t a, coordinate_t b) const noexcept
{
    return round_up(a, b);
}
coordinate_t vertical::midpoint(coordinate_t a, coordinate_t b) const noexcept
{
    return round_up(a, b);
}
coordinate_t rev_horizontal::midpoint(coordinate_t a, coordinate_t b) const noexcept
{
    return round_down(a, b);
}
coordinate_t rev_vertical::midpoint(coordinate_t a, coordinate_t b) const noexcept
{
    return round_down(a, b);
}

}
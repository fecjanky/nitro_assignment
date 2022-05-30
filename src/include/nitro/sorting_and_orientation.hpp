#pragma once

#include <cassert>
#include <nitro/fwd.hpp>
#include <nitro/rectangle.hpp>
#include <nitro/types.hpp>
#include <set>
#include <variant>

namespace nitro {

template <typename Derived> struct sorting_base {
    using is_transparent = std::true_type;

    constexpr bool operator()(const rectangle& lhs, const rectangle& rhs) const noexcept
    {
        return std::is_lt(static_cast<Derived const*>(this)->compare(lhs, rhs));
    }
    constexpr bool operator()(const rectangle& lhs, coordinate_t rhs) const noexcept
    {
        return std::is_lt(static_cast<Derived const*>(this)->compare(lhs, rhs));
    }
    constexpr bool operator()(coordinate_t lhs, const rectangle& rhs) const noexcept
    {
        return std::is_lt(static_cast<Derived const*>(this)->compare(lhs, rhs));
    }

    constexpr bool operator()(rect_ptr lhs, rect_ptr rhs) const noexcept
    {
        return (*this)(*lhs, *rhs);
    }

    constexpr bool operator()(rect_ptr lhs, coordinate_t rhs) const noexcept
    {
        return (*this)(*lhs, rhs);
    }
    constexpr bool operator()(coordinate_t lhs, rect_ptr rhs) const noexcept
    {
        return (*this)(lhs, *rhs);
    }
};
struct horizontal_sort : public sorting_base<horizontal_sort> {
    using is_transparent = std::true_type;
    std::strong_ordering compare(const rectangle& lhs, const rectangle& rhs) const noexcept;
    std::weak_ordering   compare(const rectangle& lhs, coordinate_t rhs) const noexcept;
    std::weak_ordering   compare(coordinate_t lhs, rectangle const& rhs) const noexcept;
    // coordinate_t         reference(rectangle const& r) const;
};

struct rev_horizontal_sort : public sorting_base<rev_horizontal_sort> {
    using is_transparent = std::true_type;
    std::strong_ordering compare(const rectangle& lhs, const rectangle& rhs) const noexcept;
    std::weak_ordering   compare(const rectangle& lhs, coordinate_t rhs) const noexcept;
    std::weak_ordering   compare(coordinate_t lhs, rectangle const& rhs) const noexcept;
};
struct vertical_sort : public sorting_base<vertical_sort> {
    using is_transparent = std::true_type;
    std::strong_ordering compare(const rectangle& lhs, const rectangle& rhs) const noexcept;
    std::weak_ordering   compare(const rectangle& lhs, coordinate_t rhs) const noexcept;
    std::weak_ordering   compare(coordinate_t lhs, rectangle const& rhs) const noexcept;
};

struct rev_vertical_sort : public sorting_base<rev_vertical_sort> {
    using is_transparent = std::true_type;
    std::strong_ordering compare(const rectangle& lhs, const rectangle& rhs) const noexcept;
    std::weak_ordering   compare(const rectangle& lhs, coordinate_t rhs) const noexcept;
    std::weak_ordering   compare(coordinate_t lhs, rectangle const& rhs) const noexcept;
};

using orderings
    = std::variant<horizontal_sort, vertical_sort, rev_horizontal_sort, rev_vertical_sort>;

struct ordering_t : public orderings {
    using is_transparent = std::true_type;
    template <typename T1, typename T2>
    constexpr bool operator()(const T1& lhs, const T2& rhs) const noexcept
    {
        return std::visit(
            [&](auto& ord) { return ord(lhs, rhs); }, static_cast<orderings const&>(*this));
    }
};

struct vertical : public orientation {
    [[nodiscard]] coordinate_t ref(const rectangle& r) const noexcept;
    [[nodiscard]] coordinate_t midpoint(coordinate_t, coordinate_t) const noexcept;
    [[nodiscard]] bool         inner_slice(const rectangle& r) const noexcept;
    [[nodiscard]] bool         above(coordinate_t c) const noexcept;
    [[nodiscard]] std::pair<rectangle, rectangle> slice(const rectangle& r) const noexcept;
};

struct rev_vertical : public orientation {
    [[nodiscard]] coordinate_t ref(const rectangle& r) const noexcept;
    [[nodiscard]] coordinate_t midpoint(coordinate_t, coordinate_t) const noexcept;
    [[nodiscard]] bool         inner_slice(const rectangle& r) const noexcept;
    [[nodiscard]] bool         above(coordinate_t c) const noexcept;
    [[nodiscard]] std::pair<rectangle, rectangle> slice(const rectangle& r) const noexcept;
};

struct horizontal : public orientation {
    [[nodiscard]] coordinate_t ref(const rectangle& r) const noexcept;
    [[nodiscard]] coordinate_t midpoint(coordinate_t, coordinate_t) const noexcept;
    [[nodiscard]] bool         inner_slice(const rectangle& r) const noexcept;
    [[nodiscard]] bool         above(coordinate_t c) const noexcept;
    [[nodiscard]] std::pair<rectangle, rectangle> slice(const rectangle& r) const noexcept;
};

struct rev_horizontal : public orientation {
    [[nodiscard]] coordinate_t ref(const rectangle& r) const noexcept;
    [[nodiscard]] coordinate_t midpoint(coordinate_t, coordinate_t) const noexcept;
    [[nodiscard]] bool         inner_slice(const rectangle& r) const noexcept;
    [[nodiscard]] bool         above(coordinate_t c) const noexcept;
    [[nodiscard]] std::pair<rectangle, rectangle> slice(const rectangle& r) const noexcept;
};

template <typename T> struct next_orientation;
template <> struct next_orientation<vertical> {
    using type = horizontal;
};
template <> struct next_orientation<horizontal> {
    using type = rev_vertical;
};
template <> struct next_orientation<rev_vertical> {
    using type = rev_horizontal;
};
template <> struct next_orientation<rev_horizontal> {
    using type = vertical;
};
template <typename T> using next_orientation_t = typename next_orientation<T>::type;

template <typename T> struct ordering_of;
template <> struct ordering_of<horizontal> {
    using type = vertical_sort;
};
template <> struct ordering_of<rev_horizontal> {
    using type = rev_vertical_sort;
};
template <> struct ordering_of<vertical> {
    using type = horizontal_sort;
};
template <> struct ordering_of<rev_vertical> {
    using type = rev_horizontal_sort;
};
template <typename T> using ordering_of_t = typename ordering_of<T>::type;

using sorted_rectangles = std::pmr::set<rect_ptr, ordering_t>;

template <typename Orientation, typename RectRange>
auto sorted(RectRange const& rects) -> sorted_rectangles requires RectPtrRange<RectRange>
{
    return { rng::begin(rects), rng::end(rects), ordering_t { ordering_of_t<Orientation> {} } };
}

}
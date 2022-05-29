#pragma once
#include "nitro/exceptions.hpp"
#include <bits/iterator_concepts.h>
#include <bits/ranges_base.h>
#include <compare>
#include <cstddef>
#include <cstdint>

#include <memory>
#include <nitro/fwd.hpp>
#include <nitro/utils.hpp>

#include <list>
#include <ranges>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <gsl/gsl-lite.hpp>
namespace nitro {

struct point {
    coordinate_t x {}, y {};
    auto         operator<=>(const point&) const = default;
};

struct orientation {
    coordinate_t val {};
};
struct vertical;
struct horizontal;

using rect_ptr = gsl::not_null<rectangle const*>;

struct rectangle {
    using parent_ptr   = gsl::not_null<rectangle const*>;
    using identifier_t = std::variant<std::size_t, parent_ptr>;

    constexpr explicit rectangle(point origin, point extent, identifier_t id = {})
        : m_p { origin }
        , m_w { extent.x }
        , m_h { extent.y }
        , m_parent { get_parent(id) }
        , m_id { get_id(id) }
    {
        if (m_w < 1 || m_h < 1) {
            throw invalid_arg("invalid height or width");
        }
    }
    // constexpr explicit rectangle(
    //     point origin, coordinate_t width, coordinate_t height, identifier_t id = {})
    //     : rectangle { origin, { width, height }, std::move(id) }
    // {
    // }

    [[nodiscard]] constexpr auto origin() const noexcept { return m_p; }
    [[nodiscard]] constexpr auto extent() const noexcept { return point { m_w, m_h }; }
    [[nodiscard]] constexpr auto height() const noexcept { return m_h; }
    [[nodiscard]] constexpr auto width() const noexcept { return m_w; }

    [[nodiscard]] constexpr auto id() const noexcept { return m_id; }
    [[nodiscard]] constexpr auto parent() const noexcept { return m_parent; }

    [[nodiscard]] static constexpr rectangle const* get_parent(identifier_t const& id)
    {
        return std::visit(overload { [](std::size_t id) -> rectangle const* { return nullptr; },
                              [](parent_ptr p) -> rectangle const* { return p; } },
            id);
    }

    [[nodiscard]] static constexpr std::size_t get_id(identifier_t const& id)
    {
        return std::visit(overload { [](std::size_t id) -> std::size_t { return id; },
                              [](parent_ptr p) -> std::size_t { return p->id(); } },
            id);
    };
    constexpr std::partial_ordering operator<=>(const rectangle& other) const noexcept
    {
        if (m_id != other.m_id || m_parent != other.m_parent || m_p != other.m_p || m_w != other.m_w
            || m_h != other.m_h)
            return std::partial_ordering::unordered;
        return std::partial_ordering::equivalent;
    }
    using slice_pair = std::pair<std::optional<rectangle>, std::optional<rectangle>>;
    using slice_t    = slice_pair;

    slice_t slice(vertical v) const;
    slice_t slice(horizontal h) const;

private:
    point            m_p {};
    coordinate_t     m_w {}, m_h {};
    rectangle const* m_parent {};
    std::size_t      m_id {};
};

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
    constexpr std::strong_ordering compare(
        const rectangle& lhs, const rectangle& rhs) const noexcept
    {
        return std::tuple(lhs.origin().x, lhs.origin().y, lhs.id())
            <=> std::tuple(rhs.origin().x, rhs.origin().y, rhs.id());
    }
    constexpr std::weak_ordering compare(const rectangle& lhs, coordinate_t rhs) const noexcept
    {
        return lhs.origin().x <=> rhs;
    }
    constexpr std::weak_ordering compare(coordinate_t lhs, rectangle const& rhs) const noexcept
    {
        return lhs <=> rhs.origin().x;
    }
    constexpr auto reference(rectangle const& r) const { return r.origin().x; }
};
struct vertical_sort : public sorting_base<vertical_sort> {
    using is_transparent = std::true_type;
    constexpr std::strong_ordering compare(
        const rectangle& lhs, const rectangle& rhs) const noexcept
    {
        return std::tuple(lhs.origin().y, lhs.origin().x, lhs.id())
            <=> std::tuple(rhs.origin().y, rhs.origin().x, rhs.id());
    }
    constexpr std::weak_ordering compare(const rectangle& lhs, coordinate_t rhs) const noexcept
    {
        return lhs.origin().y <=> rhs;
    }
    constexpr std::weak_ordering compare(coordinate_t lhs, rectangle const& rhs) const noexcept
    {
        return lhs <=> rhs.origin().y;
    }

    constexpr auto reference(rectangle const& r) const { return r.origin().y; }
};

using orderings = std::variant<horizontal_sort, vertical_sort>;

struct ordering_t : public orderings {
    using is_transparent = std::true_type;
    template <typename T1, typename T2>
    constexpr bool operator()(const T1& lhs, const T2& rhs) const noexcept
    {
        return std::visit(
            [&](auto& ord) { return ord(lhs, rhs); }, static_cast<orderings const&>(*this));
    }

    constexpr auto reference(rectangle const& r) const
    {
        return std::visit(
            [&](auto& ord) { return ord.reference(r); }, static_cast<orderings const&>(*this));
    }
};

enum class direction_t { S = 0, SE = 1, E = 2, NE = 3, N = 4 };

template <typename T>
concept RectPtrRange = rng::forward_range<T> && requires(T& t)
{
    {
        std::addressof(**t.begin())
        } -> std::convertible_to<rect_ptr>;
};

// template <typename T>
// auto rect_ptr_view(T& rng) requires rng::forward_range<T> && std::is_convertible_v<
//     typename rng::iterator_t<T>::value_type, rectangle const&>
// {
//     return rng
// }

static constexpr std::size_t DIR_COUNT = 5;
struct partition_tree {
    using rectangles_list   = std::list<rectangle>;
    using sorted_rectangles = std::set<rect_ptr, ordering_t>;
    using slice_t           = std::tuple<rectangles_list, sorted_rectangles, sorted_rectangles>;
    explicit partition_tree(rectangles_list lst)
        : m_rects(std::move(lst))
    {
        build();
    }
    partition_tree(const partition_tree&)            = delete;
    partition_tree& operator=(const partition_tree&) = delete;

    static slice_t slice(horizontal, sorted_rectangles const&);
    static slice_t slice(vertical, sorted_rectangles const&);

    template <typename Orientation, typename RectRange>
    static sorted_rectangles sorted(RectRange const& rects) requires RectPtrRange<RectRange>
    {
        return { rng::begin(rects), rng::end(rects), ordering_t { Orientation {} } };
    }
    struct node {
        node*             parent {};
        node*             below {};
        node*             above {};
        sorted_rectangles rects{};
    };
    using node_list = std::list<node>;

private:
    void        build();
    static void build_nodes(node* parent, node*& target, node_list& nodes, rectangles_list& rects,
        sorted_rectangles );

    node*           m_root {};
    rectangles_list m_rects;
    node_list       m_nodes;
};

struct vertical : public orientation {
    [[nodiscard]] constexpr auto ref(const rectangle& r) noexcept { return r.origin().x; }
    [[nodiscard]] constexpr auto extent(const rectangle& r) noexcept { return r.width(); }
    [[nodiscard]] constexpr std::pair<rectangle, rectangle> slice(
        const rectangle& r, coordinate_t slice_width) noexcept
    {
        return { rectangle { r.origin(), { slice_width, r.height() }, &r },
            rectangle { { r.origin().x + slice_width, r.origin().y },
                { r.width() - slice_width, r.height() }, &r } };
    }
};
struct horizontal : public orientation {
    [[nodiscard]] constexpr auto ref(const rectangle& r) noexcept { return r.origin().y; }
    [[nodiscard]] constexpr auto extent(const rectangle& r) noexcept { return r.height(); }
    [[nodiscard]] constexpr std::pair<rectangle, rectangle> slice(
        const rectangle& r, coordinate_t slice_width) noexcept
    {
        return { rectangle { r.origin(), { r.width(), slice_width }, &r },
            rectangle { { r.origin().x, r.origin().y + slice_width },
                { r.width(), r.height() - slice_width }, &r } };
    }
};

}
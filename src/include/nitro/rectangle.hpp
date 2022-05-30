#pragma once

#include <list>
#include <nitro/fwd.hpp>
#include <nitro/types.hpp>
#include <nitro/utils.hpp>

#include <gsl/gsl-lite.hpp>
#include <utility>
#include <variant>

namespace nitro {

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

    [[nodiscard]] constexpr auto origin() const noexcept { return m_p; }
    [[nodiscard]] constexpr auto extent() const noexcept { return point { m_w, m_h }; }
    [[nodiscard]] constexpr auto height() const noexcept { return m_h; }
    [[nodiscard]] constexpr auto width() const noexcept { return m_w; }
    [[nodiscard]] constexpr auto id() const noexcept { return m_id; }
    [[nodiscard]] constexpr auto parent() const noexcept { return m_parent; }
    [[nodiscard]] constexpr auto root() const noexcept
    {
        return m_parent != nullptr ? m_parent : this;
    }

    [[nodiscard]] static constexpr rectangle const* get_parent(identifier_t const& id)
    {
        return std::visit(overload { [](std::size_t id) -> rectangle const* { return nullptr; },
                              [](parent_ptr p) -> rectangle const* { return p->root(); } },
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

    slice_t slice(vertical const& v) const;
    slice_t slice(horizontal const& h) const;
    slice_t slice(rev_vertical const& rv) const;
    slice_t slice(rev_horizontal const& rh) const;

    // signature seleted to be able to use in a reduce operation
    static std::optional<rectangle> intersect(rectangle const& lhs,std::optional<rectangle> const& rhs);

private:
    point            m_p {};
    coordinate_t     m_w {}, m_h {};
    rectangle const* m_parent {};
    std::size_t      m_id {};
};

using rectangles_list = std::pmr::list<rectangle>;

}
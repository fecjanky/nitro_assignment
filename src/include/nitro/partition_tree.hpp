#pragma once
#include <bits/ranges_algo.h>

#include <nitro/exceptions.hpp>
#include <nitro/fwd.hpp>
#include <nitro/rectangle.hpp>
#include <nitro/sorting_and_orientation.hpp>
#include <nitro/utils.hpp>

#include <numeric>

#include <gsl/gsl-lite.hpp>

namespace nitro {

using slice_t = std::tuple<rectangles_list, sorted_rectangles, sorted_rectangles>;

struct partition_tree {

    static bool is_homogeneous(sorted_rectangles const&);
    explicit partition_tree(rectangles_list lst);
    partition_tree(const partition_tree&)            = delete;
    partition_tree(partition_tree&&)                 = delete;
    partition_tree& operator=(const partition_tree&) = delete;
    partition_tree& operator=(partition_tree&&)      = delete;

    static slice_t slice(horizontal, sorted_rectangles const&);
    static slice_t slice(vertical, sorted_rectangles const&);
    static slice_t slice(rev_vertical, sorted_rectangles const&);
    static slice_t slice(rev_horizontal, sorted_rectangles const&);

    struct node {
        node*             parent {};
        node*             below {};
        node*             above {};
        sorted_rectangles rects {};
    };

    auto  leaf_begin() const { return m_leaf_nodes.begin(); }
    auto  leaf_end() const { return m_leaf_nodes.end(); }
    auto& leaves() const noexcept { return m_leaf_nodes; }
    using node_list     = std::list<node>;
    using node_ptr_list = std::list<node const*>;

    struct intersection {
        static constexpr auto id_comp() noexcept
        {
            return [](auto&& lhs, auto&& rhs) { return lhs->id() < rhs->id(); };
        }
        static constexpr auto to_id() noexcept
        {
            return [](auto&& t) { return t->id(); };
        }
        template <rng::forward_range Rng>
        explicit intersection(Rng&& range)
            : m_rects(rng::begin(range), rng::end(range))
        {
            rng::sort(m_rects, std::less<> {}, to_id());
            auto [first, last] = rng::unique(m_rects, std::equal_to<> {}, to_id());
            m_rects.erase(first, last);
            if (m_rects.size() < 2) {
                throw invalid_arg("intersection must be at least between two rectangles");
            };
        }
        bool operator<(const intersection& other) const noexcept;

        auto& constituents() const noexcept { return m_rects; }

        rectangle calculate() const;

    private:
        std::vector<rect_ptr> m_rects;
    };

    std::pmr::set<intersection> intersections() const;

    template <typename orientation_type>
    static coordinate_t split_point(sorted_rectangles const& sorted_rects);

private:
    void        build();
    static void build_nodes(node* parent, node*& target, node_list& nodes, node_ptr_list& leaves,
        rectangles_list& rects, sorted_rectangles&&);

    node*           m_root {};
    rectangles_list m_rects;
    node_list       m_nodes;
    node_ptr_list   m_leaf_nodes;
};

template <typename orientation_type>
coordinate_t partition_tree::split_point(sorted_rectangles const& sorted_rects)
{
    assert(!sorted_rects.empty());
    auto&      first      = *sorted_rects.begin();
    auto&      last       = *std::prev(sorted_rects.end());
    const auto first_ref  = orientation_type {}.ref(*first);
    const auto last_ref   = orientation_type {}.ref(*last);
    const auto mid_p      = orientation_type {}.midpoint(first_ref, last_ref);
    auto       split_rect = sorted_rects.lower_bound(mid_p);
    if (split_rect == sorted_rects.end())
        return mid_p;
    const auto split_point = orientation_type {}.ref(**split_rect);
    return split_point;
}
}
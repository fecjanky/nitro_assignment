#include "nitro/exceptions.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <nitro/geometry.hpp>
#include <numeric>
#include <optional>
#include <variant>

namespace nitro {

void partition_tree::build()
{
    sorted_rectangles sorted_rects(ordering_t { horizontal_sort {} });
    rng::copy(m_rects | views::transform(address_of_f {}),
        std::inserter(sorted_rects, sorted_rects.begin()));

    build_nodes(nullptr, m_root, m_nodes, m_rects, std::move(sorted_rects));
}

void partition_tree::build_nodes(node* parent, node*& target, node_list& nodes,
    rectangles_list& rects, sorted_rectangles sorted_rects)
{

    if (sorted_rects.empty()) {
        target = nullptr;
    }
    auto& new_node  = *nodes.insert(nodes.begin(), {});
    new_node.parent = parent;

    // find leftmost rectangle
    auto&      first       = *sorted_rects.begin();
    auto&      last        = *std::prev(sorted_rects.end());
    const auto first_ref   = vertical {}.ref(*first);
    const auto last_ref    = vertical {}.ref(*last);
    const auto mid_p       = std::midpoint(first_ref, last_ref);
    auto       lb          = sorted_rects.lower_bound(mid_p);
    auto       split_rect  = lb == sorted_rects.end() ? std::prev(lb) : lb;
    const auto split_point = horizontal {}.ref(**split_rect);

    auto&& [new_rects, below, above] = slice(vertical { split_point }, sorted_rects);
    rects.splice(rects.begin(), std::move(new_rects));
    if (below.empty() || above.empty()) {
        auto& non_empty = below.empty() ? (above) : (below);
        auto& target    = below.empty() ? (new_node.above) : (new_node.below);
        auto  ub        = non_empty.upper_bound(horizontal {}.ref(**non_empty.begin()));
        if (ub == non_empty.end()) {
            new_node.rects = std::move(sorted_rects);
            return;
        }
        build_nodes(std::addressof(new_node), target, nodes, rects, std::move(non_empty));
        return;
    }
    build_nodes(std::addressof(new_node), new_node.below, nodes, rects, std::move(below));
    build_nodes(std::addressof(new_node), new_node.above, nodes, rects, std::move(above));
}

template <typename SlicePolicy>
rectangle::slice_t slice_impl(SlicePolicy policy, rectangle const& r)
{
    const auto ref        = policy.ref(r);
    const auto extent     = policy.extent(r);
    const auto extent_val = ref + extent;

    // internal slice
    if (ref < policy.val && policy.val < extent_val) {
        const auto slice_width = policy.val - ref;
        return policy.slice(r, slice_width);
    }
    // on edge slice
    if (policy.val <= ref) {
        return { std::nullopt, r };
    }
    return { r, std::nullopt };
}
auto rectangle::slice(vertical v) const -> slice_t { return slice_impl(v, *this); }
auto rectangle::slice(horizontal h) const -> slice_t { return slice_impl(h, *this); }

template <typename HintF>
void add_rect(std::optional<rectangle> const& r, std::list<rectangle>& rect_list,
    partition_tree::sorted_rectangles& rects, HintF hint)
{
    if (r) {
        auto nr = rect_list.insert(rect_list.end(), *r);
        rects.insert(hint(rects), std::addressof(*nr));
    }
}

template <typename Orientation, typename ExpectedOrdering>
auto space_slice_impl(Orientation            orientation, type_tag<ExpectedOrdering>,
    partition_tree::sorted_rectangles const& rects) -> partition_tree::slice_t
{
    if (!std::holds_alternative<ExpectedOrdering>(rects.key_comp()))
        throw invalid_arg("invalid sorting of rectangles");
    const auto                        from = rects.lower_bound(orientation.val);
    std::list<rectangle>              rect_list;
    partition_tree::sorted_rectangles below(rects.key_comp());
    partition_tree::sorted_rectangles above(from, rects.end(), rects.key_comp());
    for (auto it = rects.begin(); it != from; ++it) {
        auto [r_below, r_above] = (*it)->slice(orientation);
        add_rect(r_below, rect_list, below, [](auto& s) { return s.end(); });
        add_rect(r_above, rect_list, above, [](auto& s) { return s.begin(); });
    }
    return { std::move(rect_list), std::move(below), std::move(above) };
}

auto partition_tree::slice(horizontal h, sorted_rectangles const& rects) -> slice_t
{
    return space_slice_impl(h, type_tag<vertical_sort> {}, rects);
}

auto partition_tree::slice(vertical v, sorted_rectangles const& rects) -> slice_t
{
    return space_slice_impl(v, type_tag<horizontal_sort> {}, rects);
}

}

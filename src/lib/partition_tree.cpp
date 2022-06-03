#include "nitro/exceptions.hpp"
#include "nitro/fwd.hpp"
#include "nitro/rectangle.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <memory>
#include <nitro/partition_tree.hpp>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <variant>

namespace nitro {

using pt = partition_tree;

template <typename orientation_type>
void build_nodes_impl(pt::tp start, std::optional<pt::secs> const& to, rectangles_list& rects,
    sorted_rectangles&& sorted_rects, pt::intersection_set& intersections);

void partition_tree::build()
{
    sorted_rectangles sorted_rects(ordering_t { ordering_of_t<vertical> {} });
    rng::copy(m_rects | views::transform(address_of_f {}),
        std::inserter(sorted_rects, sorted_rects.begin()));

    return build_nodes_impl<vertical>(
        m_start_time, m_timeout, m_rects, std::move(sorted_rects), m_intersections);
}

void add_leaf_node(sorted_rectangles&& rects, pt::intersection_set& intersections)
{
    if (rects.size() > 1)
        intersections.emplace(std::move(rects));
}
template <typename Next_Orientation>
void change_orientation(Next_Orientation, pt::tp start, std::optional<pt::secs> const& to,
    rectangles_list& rects, sorted_rectangles&& above, pt::intersection_set& intersections)
{
    if (pt::is_homogeneous(above)) {
        add_leaf_node(std::move(above), intersections);
        return;
    }
    sorted_rectangles ro_sorted_rects = sorted<Next_Orientation>(above);
    build_nodes_impl<Next_Orientation>(start, to, rects, std::move(ro_sorted_rects), intersections);
}

template <typename orientation_type>
void build_nodes_impl(pt::tp start, std::optional<pt::secs> const& to, rectangles_list& rects,
    sorted_rectangles&& sorted_rects, pt::intersection_set& intersections)
{
    if (to && pt::clock::now() > start + *to) {
        throw timeout("Calculation timed out ...");
    }
    if (sorted_rects.size() < 2) {
        return;
    }

    // if intersection has already been discovered then don't continue
    if (intersections.find(pt::intersection(std::as_const(sorted_rects))) != intersections.end()) {
        return;
    }

    // find leftmost rectangle
    const auto split_pt = pt::split_point<orientation_type>(sorted_rects);
    // slice according to current orientation
    auto [new_rects, below, above] = pt::slice(orientation_type { split_pt }, sorted_rects);
    // above can't be empty, as the split point is the lower bound of the mid point, which in
    //  case of a single element will be it's own origin, resulting in it being above the split line
    assert(!above.empty());
    // obtain ownership of newly created rectangles
    rects.splice(rects.begin(), std::move(new_rects));
    if (below.empty()) {
        // if there are no new splits we can continue with the next orientation
        assert(above.size() == sorted_rects.size());
        change_orientation(next_orientation_t<orientation_type> {}, start, to, rects,
            std::move(above), intersections);

        return;
    }
    // else continue recursively on both children
    build_nodes_impl<orientation_type>(start, to, rects, std::move(below), intersections);
    build_nodes_impl<orientation_type>(start, to, rects, std::move(above), intersections);
}

template <typename HintF>
void add_rect(std::optional<rectangle> const& r, rectangles_list& rect_list,
    sorted_rectangles& rects, HintF hint)
{
    if (r) {
        auto nr = rect_list.insert(rect_list.end(), *r);
        rects.insert(hint(rects), std::addressof(*nr));
    }
}

template <typename orientation_type, typename ExpectedOrdering>
auto slice_ordered_impl(orientation_type orientation, type_tag<ExpectedOrdering>,
    sorted_rectangles const&             rects) -> slice_t
{
    if (!std::holds_alternative<ExpectedOrdering>(rects.key_comp()))
        throw invalid_arg("invalid sorting of rectangles");
    const auto        from = rects.lower_bound(orientation.val);
    rectangles_list   rect_list;
    sorted_rectangles below(rects.key_comp());
    sorted_rectangles above(from, rects.end(), rects.key_comp());
    for (auto it = rects.begin(); it != from; ++it) {
        auto [r_below, r_above] = (*it)->slice(orientation);
        add_rect(r_below, rect_list, below, [](auto& s) { return s.end(); });
        add_rect(r_above, rect_list, above, [](auto& s) { return s.begin(); });
    }
    return { std::move(rect_list), std::move(below), std::move(above) };
}

auto partition_tree::slice(horizontal h, sorted_rectangles const& rects) -> slice_t
{
    return slice_ordered_impl(h, type_tag<vertical_sort> {}, rects);
}

auto partition_tree::slice(vertical v, sorted_rectangles const& rects) -> slice_t
{
    return slice_ordered_impl(v, type_tag<horizontal_sort> {}, rects);
}
auto partition_tree::slice(rev_vertical v, sorted_rectangles const& rects) -> slice_t
{
    return slice_ordered_impl(v, type_tag<rev_horizontal_sort> {}, rects);
}
auto partition_tree::slice(rev_horizontal v, sorted_rectangles const& rects) -> slice_t
{
    return slice_ordered_impl(v, type_tag<rev_vertical_sort> {}, rects);
}

bool partition_tree::is_homogeneous(sorted_rectangles const& rects)
{

    if (rects.empty())
        return true;
    const auto& ref = **rects.begin();
    return rng::all_of(rects, [&](const auto& pr) { return ref.extent() == pr->extent(); });
}

bool partition_tree::intersection::operator<(const intersection& other) const noexcept
{
    if (m_rects.size() < other.m_rects.size())
        return true;
    if (m_rects.size() > other.m_rects.size())
        return false;
    return rng::lexicographical_compare(m_rects, other.m_rects, std::less<>(), to_id(), to_id());
}

bool partition_tree::intersection::operator==(const intersection& other) const noexcept
{
    return m_rects.size() == other.m_rects.size() && m_rects == other.m_rects;
}

partition_tree::partition_tree(rectangles_list lst, std::optional<secs> timeout)
    : m_rects(std::move(lst))
    , m_start_time(std::chrono::system_clock::now())
    , m_timeout(timeout)
{
    build();
}

auto partition_tree::intersections() const -> intersection_set const&
{
    return m_intersections;
}

rectangle partition_tree::intersection::calculate() const
{
    auto first_elem = *m_rects.begin();

    auto result = std::accumulate(std::next(m_rects.begin()), m_rects.end(),
        std::make_optional(*first_elem->root()),
        [](std::optional<rectangle> rect, rect_ptr p) -> std::optional<rectangle> {
            return rectangle::intersect(*p->root(), rect);
        });
    if (!result)
        throw std::logic_error("must have intersections...");
    return *result;
}

}

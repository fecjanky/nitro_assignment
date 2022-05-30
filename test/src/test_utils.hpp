#include <nitro/fwd.hpp>
#include <nitro/rectangle.hpp>
#include <nitro/sorting_and_orientation.hpp>

#include <type_traits>
#include <vector>

template <typename T> auto id(T t)
{
    if constexpr (std::is_integral_v<T>) {
        return nitro::rectangle::identifier_t(static_cast<size_t>(t));
    } else
        return nitro::rectangle::identifier_t(t);
}

inline auto rect_vec(nitro::sorted_rectangles const& sr)
{
    std::vector<nitro::rectangle> out;
    for (auto p : sr)
        out.push_back(*p);
    return out;
}
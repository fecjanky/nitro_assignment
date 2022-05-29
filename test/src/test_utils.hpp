#include <nitro/geometry.hpp>

#include <type_traits>

template <typename T> auto id(T t)
{
    if constexpr (std::is_integral_v<T>) {
        return nitro::rectangle::identifier_t(static_cast<size_t>(t));
    } else
        return nitro::rectangle::identifier_t(t);
}
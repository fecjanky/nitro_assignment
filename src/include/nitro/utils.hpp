#pragma once

#include <memory>
namespace nitro {
template <typename... F> struct overload : public F... {
    using F::operator()...;
};

template <typename... F> overload(F...) -> overload<F...>;

struct address_of_f {
    template <typename T> constexpr auto operator()(T& v) const noexcept
    {
        return std::addressof(v);
    }
};

template <typename T> struct type_tag {
    using type = T;
};
}
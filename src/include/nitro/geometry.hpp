#pragma once
#include <cstdint>

#include <nitro/fwd.hpp>
#include <stdexcept>

namespace nitro {

struct point {
    coordinate_t x {}, y {};
};

struct rectangle {
    constexpr explicit rectangle(point origin, coordinate_t w, coordinate_t h)
        : m_p { origin }
        , m_w { w }
        , m_h { h }
    {
        if (m_w < 1 || m_h < 1) {
            throw invalid_arg("invalid height or width");
        }
    }

    [[nodiscard]] constexpr auto origin() const noexcept { return m_p; }
    [[nodiscard]] constexpr auto height() const noexcept { return m_h; }
    [[nodiscard]] constexpr auto width() const noexcept { return m_w; }

private:
    point        m_p {};
    coordinate_t m_w {}, m_h {};
};
}
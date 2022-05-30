#include "nitro/fwd.hpp"
#include "nitro/rectangle.hpp"
#include <nitro/io.hpp>
#include <nlohmann/json_fwd.hpp>
#include <ranges>
namespace nitro {

rectangle to_rectangle(nlohmann::json j, std::size_t id)
{
    return rectangle { { j["x"].get<coordinate_t>(), j["y"].get<coordinate_t>() },
        { j["w"].get<coordinate_t>(), j["h"].get<coordinate_t>() }, id };
}

rectangles_list to_rectangles(nlohmann::json j)
{
    const auto&     arr = j["rects"].get<nlohmann::json::array_t>();
    rectangles_list result;
    std::size_t     cnt { 1 };
    for (const auto& v : arr) {
        result.push_back(to_rectangle(v, cnt++));
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const point& p)
{
    return os << '(' << p.x << ',' << p.y << ')';
}
std::ostream& operator<<(std::ostream& os, const rectangle& r)
{
    return os << '\t' << r.id() << ": Rectangle at " << r.origin() << ", w=" << r.width()
              << ", h=" << r.height() << '.';
}

}
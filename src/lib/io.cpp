#include "nitro/fwd.hpp"
#include <nitro/io.hpp>

namespace nitro {

rectangle to_rectangle(nlohmann::json j)
{
    return rectangle { { j["x"].get<coordinate_t>(), j["y"].get<coordinate_t>() },
        j["w"].get<coordinate_t>(), j["h"].get<coordinate_t>() };
}

}
#pragma once
#include <nitro/fwd.hpp>
#include <nitro/geometry.hpp>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace nitro {

[[nodiscard]] rectangle to_rectangle(nlohmann::json);

}
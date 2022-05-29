#pragma once
#include <nitro/fwd.hpp>
#include <nitro/geometry.hpp>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <ostream>
#include <vector>
namespace nitro {

[[nodiscard]] rectangle              to_rectangle(nlohmann::json, std::size_t id = 0);
[[nodiscard]] std::vector<rectangle> to_rectangles(nlohmann::json);

std::ostream& operator<<(std::ostream& os, const point& p);
std::ostream& operator<<(std::ostream& os, const rectangle& p);

}
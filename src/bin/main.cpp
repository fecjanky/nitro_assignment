
#include "nitro/exceptions.hpp"
#include "nitro/fwd.hpp"
#include "nitro/rectangle.hpp"
#include <fstream>
#include <iostream>
#include <nitro/nitro.hpp>

#include <exception>
#include <nlohmann/json_fwd.hpp>

auto open_file(auto&& path)
{
    std::ifstream ifs(path);
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    return ifs;
}

std::ostream& print_input(std::ostream& os, const nitro::rectangles_list rects)
{
    os << "Input:\n";
    for (const auto& r : rects)
        os << r << '\n';
    os << '\n';
    return os;
}

std::ostream& print_intersection(std::ostream& os, const nitro::partition_tree::intersection& i)
{
    const auto r = i.calculate();

    os << '\t' << "Between rectangle ";
    auto last_elem_it = std::prev(i.constituents().end());
    for (auto it = i.constituents().begin(); it != i.constituents().end(); ++it) {
        os << (*it)->id();
        if (it != last_elem_it)
            os << (std::next(it) != last_elem_it ? ", " : " and ");
    }
    os << " at " << r.origin() << ", w=" << r.width() << ", h=" << r.height() << '.';
    return os;
}

std::ostream& print_output(
    std::ostream& os, const std::pmr::set<nitro::partition_tree::intersection>& interections)
{
    std::cout << "Intersections\n";
    for (const auto& i : interections) {
        print_intersection(std::cout, i);
        os << '\n';
    }
    return os;
}

int main(int argc, char* argv[])
try {
    if (argc < 2)
        throw nitro::invalid_arg("missing JSON file input");
    auto ifs          = open_file(argv[1]);
    auto jtext        = nlohmann::json::parse(ifs);
    auto old_resource = std::pmr::get_default_resource();
    auto pool         = nitro::get_default_memory_resource(old_resource);
    std::pmr::set_default_resource(&pool);
    auto rects = nitro::to_rectangles(jtext);
    print_input(std::cout, rects);
    nitro::partition_tree pt(std::move(rects));
    auto                  interections = pt.intersections();
    print_output(std::cout, interections);

} catch (const std::exception& ex) {
    return -1;
} catch (...) {
    return -2;
}
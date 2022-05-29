#pragma once
#include <stdexcept>

namespace nitro {
struct invalid_arg : public std::invalid_argument {
    using std::invalid_argument::invalid_argument;
};
}
#pragma once

#include <memory_resource>

namespace nitro {

inline auto get_default_memory_resource(
    std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
{
    std::pmr::pool_options opts { .max_blocks_per_chunk = 65536,
        .largest_required_pool_block                    = 1024 };
    return std::pmr::unsynchronized_pool_resource(opts, upstream);
}
}
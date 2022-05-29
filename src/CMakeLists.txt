cmake_minimum_required(VERSION 3.1.3)


project("nitro assignment" CXX)

set (LIB_SRC  lib/io.cpp lib/geometry.cpp)

add_library(nitro_lib STATIC ${LIB_SRC})
find_package(nlohmann_json)
find_package(gsl-lite)
target_link_libraries(nitro_lib PUBLIC nlohmann_json::nlohmann_json  gsl::gsl-lite)
target_include_directories(nitro_lib PUBLIC "include" PRIVATE "include_priv")
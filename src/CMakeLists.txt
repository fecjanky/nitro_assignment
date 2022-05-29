cmake_minimum_required(VERSION 3.1.3)


project("nitro assignment" CXX)

file(GLOB_RECURSE LIB_SRC CONFIGURE_DEPENDS "lib/*.cpp")

add_library(nitro_lib STATIC ${LIB_SRC})
find_package(nlohmann_json)
target_link_libraries(nitro_lib PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(nitro_lib PUBLIC "include" PRIVATE "include_priv")
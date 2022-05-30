cmake_minimum_required(VERSION 3.1.3)


project("nitro assignment" CXX)
enable_testing()

set (LIB_SRC  lib/io.cpp lib/partition_tree.cpp lib/rectangle.cpp lib/sorting_and_orientation.cpp)

add_library(nitro_lib STATIC ${LIB_SRC})
find_package(nlohmann_json)
find_package(gsl-lite)
target_link_libraries(nitro_lib PUBLIC nlohmann_json::nlohmann_json  gsl::gsl-lite)
target_include_directories(nitro_lib PUBLIC "include")

set (BIN_SRC  bin/main.cpp)
add_executable(nitro_app  ${BIN_SRC})
target_link_libraries(nitro_app PRIVATE nitro_lib)

add_test("functional"  nitro_app ${CMAKE_CURRENT_SOURCE_DIR}/../test/data/sample.json )
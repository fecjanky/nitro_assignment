cmake_minimum_required(VERSION 3.1.3)


enable_testing()

find_package(Catch2)

project("test binaries" CXX)

set(FILES "src/unit.cpp")
add_executable("unit_test" ${FILES})
target_link_libraries("unit_test" Catch2::Catch2  nitro_lib)

add_test(NAME "unit"  COMMAND unit_test)


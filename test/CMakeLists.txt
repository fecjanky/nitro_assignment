cmake_minimum_required(VERSION 3.1.3)


enable_testing()

find_package(Catch2)

project("test binaries" CXX)

set(FILES "src/main.cpp" "src/test_rectangle.cpp" "src/test_partition_tree.cpp")
add_executable("unit_test" ${FILES})
target_link_libraries("unit_test" Catch2::Catch2  nitro_lib)
add_test( "unit" unit_test "~[slow]" -d yes )
add_test( NAME "unit_release" COMMAND unit_test "[slow]" -d yes  CONFIGURATIONS Release )



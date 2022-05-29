cmake_minimum_required(VERSION 3.1.3)

project("nitro assignment" VERSION 0.1 LANGUAGES CXX)

enable_testing()

option(DEV_BUILD "development build, e.g. add headers to solution on MSVC, etc.")

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

if(MSVC)
set(CMAKE_CXX_STANDARD 23)
else()
set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

add_subdirectory(src)
add_subdirectory(test)



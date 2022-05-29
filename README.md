# nitro_assignment


## build
mkdir build
cd build
conan install .. --install-folder ./deps --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=deps/conan_toolchain.cmake
cmake --build .
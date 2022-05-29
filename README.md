# nitro_assignment


## build & test
mkdir build
cd build
conan install .. --build=missing
conan profile new default --detect  # Generates default profile detecting GCC and sets old ABI
conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
cmake ..
cmake --build .
ctest 
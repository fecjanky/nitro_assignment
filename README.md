# nitro_assignment

Please see attached [document](spec.pdf) for the specification

## build & test
* create build director
```bash
mkdir build
cd build
```
* install dependecny packages (`pip install conan` if missing)
```bash
conan install .. --build=missing
conan profile new default --detect  # Generates default profile detecting GCC and sets old ABI
conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
```

* generate cmake build files
```bash
cmake .. -DCMAKE_BUILD_TYPE=<Build type>
cmake --build .
```
* Following build types are supported: `Debug Release RelWithDebInfo MinSizeRel Coverage Sanitize UBSAN TSAN`
* execute tests
```bash
ctest .
```
* generate coverage report in case of `Coverage build`
```bash
./scripts/genreport.sh
```
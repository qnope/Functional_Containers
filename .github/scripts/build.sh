set -e
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFC_BUILD_TESTS=ON
cmake --build build --config Release -j 8 -v
cd build/Tests/
ctest -C Release -VV

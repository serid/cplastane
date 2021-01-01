cd cmake-build-debug
cmake --build . --target clean -- -j 4
cmake --build . --target all -- -j 4
cd ..
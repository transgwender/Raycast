mkdir -p build
cd build || exit
cmake ..
make
./raycast

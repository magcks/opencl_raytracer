# Check if the project still builds
mkdir -p build > /dev/null 2&>/dev/null
cd build
cmake ..
make -j
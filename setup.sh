mkdir -p build   # Create the build directory if it doesn't exist
cd build         # Navigate into the build directory
cmake ..         # Run CMake from the parent directory
make             # Compile the project
./raycast        # Run the resulting executable

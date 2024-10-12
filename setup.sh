#!/bin/bash

# File to store the previous checksum
CHECKSUM_FILE=".cmake_checksum"

# Get the current checksum of the CMakeLists.txt and important files
CURRENT_CHECKSUM=$(md5 -q CMakeLists.txt)  # For macOS

# Check if the checksum file exists
if [ -f $CHECKSUM_FILE ]; then
    PREVIOUS_CHECKSUM=$(cat $CHECKSUM_FILE)
else
    PREVIOUS_CHECKSUM=""
fi

# If the checksums differ, delete the build directory and regenerate it
if [ "$CURRENT_CHECKSUM" != "$PREVIOUS_CHECKSUM" ]; then
    echo "Detected changes in CMakeLists.txt, cleaning build..."
    rm -rf build   # Delete the entire build directory
    mkdir build    # Recreate the build directory
    echo $CURRENT_CHECKSUM > $CHECKSUM_FILE  # Update the checksum file
else
    echo "No major changes detected."
fi

# Continue with the build process
cd build
cmake ..
make
./raycast

#!/bin/bash

# Simple build script for GPU Music Player

echo "Building GPU Music Player..."

# Create build directory if it doesn't exist
mkdir -p build

cd build

# Configure with CMake (enable all features)
cmake .. \
    -DENABLE_CUDA=ON \
    -DENABLE_OPENCL=ON \
    -DENABLE_VULKAN=ON \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON

# Build the project
make -j$(nproc)

echo "Build completed successfully!"

cd ..

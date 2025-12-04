@echo off

REM Simple build script for GPU Music Player (Windows)

echo Building GPU Music Player...

REM Create build directory if it doesn't exist
mkdir build 2>nul
cd build

REM Configure with CMake (enable all features)
cmake .. ^
    -DENABLE_CUDA=ON ^
    -DENABLE_OPENCL=ON ^
    -DENABLE_VULKAN=ON ^
    -DBUILD_TESTS=ON ^
    -DBUILD_EXAMPLES=ON

REM Build the project
cmake --build . --config Release -j

echo.
echo Build completed successfully!

cd ..

pause
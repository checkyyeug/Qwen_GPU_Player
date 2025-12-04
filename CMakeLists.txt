cmake_minimum_required(VERSION 3.16)
project(GPUMusicPlayer)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add vcpkg toolchain if available
if(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

# Add include directories
include_directories(include)
include_directories(src)

# Add definitions for audio format support
option(ENABLE_FLAC "Enable FLAC support" ON)

if(ENABLE_FLAC)
    find_package(FLAC CONFIG QUIET)
    if(FLAC_FOUND)
        target_compile_definitions(gpu_player PRIVATE ENABLE_FLAC=1)
        target_link_libraries(gpu_player FLAC::FLAC)
        message(STATUS "FLAC support enabled")
    else()
        # Try to find library the traditional way
        find_library(FLAC_LIB FLAC)
        find_path(FLAC_INCLUDE_DIR FLAC/all.h)

        if(FLAC_LIB AND FLAC_INCLUDE_DIR)
            target_compile_definitions(gpu_player PRIVATE ENABLE_FLAC=1)
            target_link_libraries(gpu_player ${FLAC_LIB})
            target_include_directories(gpu_player PRIVATE ${FLAC_INCLUDE_DIR})
            message(STATUS "FLAC support enabled (traditional find)")
        else()
            message(WARNING "FLAC library not found. FLAC support will be disabled.")
            set(ENABLE_FLAC OFF)
        endif()
    endif()
endif()

# Add definitions for GPU support
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_OPENCL "Enable OpenCL support" OFF)
option(ENABLE_VULKAN "Enable Vulkan support" OFF)

if(ENABLE_CUDA)
    find_package(CUDAToolkit QUIET)
    if(CUDAToolkit_FOUND)
        target_compile_definitions(gpu_player PRIVATE ENABLE_CUDA=1)
        enable_language(CUDA)
    endif()
endif()

if(ENABLE_OPENCL)
    find_package(OpenCL QUIET)
    if(OpenCL_FOUND)
        target_compile_definitions(gpu_player PRIVATE ENABLE_OPENCL=1)
        target_link_libraries(gpu_player ${OpenCL_LIBRARIES})
        target_include_directories(gpu_player PRIVATE ${OpenCL_INCLUDE_DIRS})
    endif()
endif()

if(ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        target_compile_definitions(gpu_player PRIVATE ENABLE_VULKAN=1)
        target_link_libraries(gpu_player ${Vulkan_LIBRARIES})
        target_include_directories(gpu_player PRIVATE ${Vulkan_INCLUDE_DIRS})
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)

# For now, we'll skip FFmpeg dependency check and use a simple approach for testing

# Add source files - explicitly list all the necessary ones
set(SOURCES
    src/main.cpp
    src/core/AudioEngine.cpp
    src/core/CommandLineInterface.cpp
    src/decoders/DecoderFactory.cpp
    src/decoders/MP3Decoder.cpp
    src/gpu/GPUProcessorFactory.cpp
    src/audio/AudioDeviceDriver.cpp
)

# Create executable
add_executable(gpu_player ${SOURCES})

# Link Windows libraries for GPU detection and audio playback
if(WIN32)
    target_link_libraries(gpu_player setupapi.lib gdi32.lib winmm.lib)
endif()
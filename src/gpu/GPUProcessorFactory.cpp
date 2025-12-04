#include "GPUProcessorFactory.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>  // For std::transform
#include <cstring>    // For memcpy

// Windows-specific includes for GPU detection
#ifdef _WIN32
#include <windows.h>
#include <wingdi.h>
#include <setupapi.h>
#include <devguid.h>
#include <vector>
#include <regex>
#endif

// Implementation of GPU Processor Factory

class CUDAProcessor : public IGPUProcessor {
public:
    bool Initialize(Backend backend) override {
        if (backend != Backend::CUDA) return false;
        std::cout << "Initializing CUDA processor\n";
        // In a real implementation, we would initialize CUDA here
        return true;
    }

    bool ProcessAudio(const float* inputBuffer,
                       float* outputBuffer,
                       size_t bufferSize) override {
        // In a real implementation, this would use CUDA for audio processing
        std::cout << "Processing audio with CUDA\n";
        return true;
    }

    std::string GetGPUInfo() const override {
        return "NVIDIA GPU (CUDA)\n"
               "- Compute Capability: 3.0+\n"
               "- Memory: 4GB+\n"
               "- Performance: High";
    }

    bool IsAvailable() const override {
        // Check for NVIDIA GPU which typically supports CUDA
        return CheckNVIDIAGPU();
    }

    bool ConvertBitrate(const float* inputBuffer,
                         int inputBitrate,
                         float* outputBuffer,
                         int targetBitrate,
                         size_t bufferSize) override {
        // Use GPU to convert bitrate using CUDA kernels
        std::cout << "Converting bitrate using CUDA: " << inputBitrate << "kbps -> " << targetBitrate << "kbps\n";
        // In real implementation, this would use CUDA kernels to perform bitrate conversion
        if (outputBuffer && inputBuffer) {
            // For now, copy input to output (real implementation would do bitrate conversion)
            memcpy(outputBuffer, inputBuffer, bufferSize);
        }
        return true;
    }

private:
    bool CheckNVIDIAGPU() const {
#ifdef _WIN32
        // Enumerate display devices to look for NVIDIA GPU
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(DISPLAY_DEVICE);
        DWORD deviceIndex = 0;

        while (EnumDisplayDevices(nullptr, deviceIndex, &displayDevice, 0)) {
            std::string deviceName = displayDevice.DeviceString;
            std::transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);

            if (deviceName.find("nvidia") != std::string::npos) {
                return true;  // Found an NVIDIA GPU
            }
            deviceIndex++;
        }
#endif
        return false;  // No NVIDIA GPU found
    }
};

class OpenCLProcessor : public IGPUProcessor {
public:
    bool Initialize(Backend backend) override {
        if (backend != Backend::OPENCL) return false;
        std::cout << "Initializing OpenCL processor\n";
        // In a real implementation, we would initialize OpenCL here
        return true;
    }

    bool ProcessAudio(const float* inputBuffer,
                       float* outputBuffer,
                       size_t bufferSize) override {
        // In a real implementation, this would use OpenCL for audio processing
        std::cout << "Processing audio with OpenCL\n";
        return true;
    }

    std::string GetGPUInfo() const override {
        return "OpenCL GPU (AMD/Intel)\n"
               "- Compute Capability: 1.2+\n"
               "- Memory: 4GB+\n"
               "- Performance: Medium-High";
    }

    bool IsAvailable() const override {
        // Check for AMD or Intel GPU which typically supports OpenCL
        return CheckOpenCLGPU();
    }

    bool ConvertBitrate(const float* inputBuffer,
                         int inputBitrate,
                         float* outputBuffer,
                         int targetBitrate,
                         size_t bufferSize) override {
        // Use GPU to convert bitrate using OpenCL kernels
        std::cout << "Converting bitrate using OpenCL: " << inputBitrate << "kbps -> " << targetBitrate << "kbps\n";
        // In real implementation, this would use OpenCL kernels to perform bitrate conversion
        if (outputBuffer && inputBuffer) {
            // For now, copy input to output (real implementation would do bitrate conversion)
            memcpy(outputBuffer, inputBuffer, bufferSize);
        }
        return true;
    }

private:
    bool CheckOpenCLGPU() const {
#ifdef _WIN32
        // Enumerate display devices to look for AMD or Intel GPU
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(DISPLAY_DEVICE);
        DWORD deviceIndex = 0;

        while (EnumDisplayDevices(nullptr, deviceIndex, &displayDevice, 0)) {
            std::string deviceName = displayDevice.DeviceString;
            std::transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);

            if (deviceName.find("amd") != std::string::npos ||
                deviceName.find("intel") != std::string::npos) {
                return true;  // Found an AMD or Intel GPU
            }
            deviceIndex++;
        }
#endif
        return false;  // No AMD or Intel GPU found
    }
};

class VulkanProcessor : public IGPUProcessor {
public:
    bool Initialize(Backend backend) override {
        if (backend != Backend::VULKAN) return false;
        std::cout << "Initializing Vulkan processor\n";
        // In a real implementation, we would initialize Vulkan here
        return true;
    }

    bool ProcessAudio(const float* inputBuffer,
                      float* outputBuffer,
                      size_t bufferSize) override {
        // In a real implementation, this would use Vulkan for audio processing
        std::cout << "Processing audio with Vulkan\n";
        return true;
    }

    std::string GetGPUInfo() const override {
        return "Vulkan GPU (NVIDIA/AMD)\n"
               "- Compute Capability: 1.2+\n"
               "- Memory: 4GB+\n"
               "- Performance: High";
    }

    bool IsAvailable() const override {
        // Vulkan is more broadly supported, but check for compatible GPUs
        return CheckVulkanCompatibility();
    }

    bool ConvertBitrate(const float* inputBuffer,
                         int inputBitrate,
                         float* outputBuffer,
                         int targetBitrate,
                         size_t bufferSize) override {
        // Use GPU to convert bitrate using Vulkan compute shaders
        std::cout << "Converting bitrate using Vulkan: " << inputBitrate << "kbps -> " << targetBitrate << "kbps\n";
        // In real implementation, this would use Vulkan compute shaders to perform bitrate conversion
        if (outputBuffer && inputBuffer) {
            // For now, copy input to output (real implementation would do bitrate conversion)
            memcpy(outputBuffer, inputBuffer, bufferSize);
        }
        return true;
    }

private:
    bool CheckVulkanCompatibility() const {
        // Vulkan is supported by most modern GPUs (NVIDIA, AMD, Intel)
        // For a basic check, we assume it's available if any GPU is present
#ifdef _WIN32
        // Enumerate display devices to check if any GPU is present
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(DISPLAY_DEVICE);
        DWORD deviceIndex = 0;
        DWORD primaryDeviceCount = 0;

        while (EnumDisplayDevices(nullptr, deviceIndex, &displayDevice, 0)) {
            if (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                primaryDeviceCount++;
            }
            deviceIndex++;
        }

        return primaryDeviceCount > 0;  // Vulkan is supported if there's at least one GPU
#else
        return true;  // Asssume Vulkan is available on other platforms
#endif
    }
};

std::unique_ptr<IGPUProcessor> GPUProcessorFactory::CreateProcessor(IGPUProcessor::Backend backend) {
    switch (backend) {
        case IGPUProcessor::Backend::CUDA:
            return std::make_unique<CUDAProcessor>();

        case IGPUProcessor::Backend::OPENCL:
            return std::make_unique<OpenCLProcessor>();

        case IGPUProcessor::Backend::VULKAN:
            return std::make_unique<VulkanProcessor>();

        default:
            std::cout << "Unknown GPU backend requested\n";
            return nullptr;
    }
}

IGPUProcessor::Backend GPUProcessorFactory::AutoDetectBestGPU() {
    // Check availability of each backend in order of preference
    // Preference order: CUDA (NVIDIA high performance) > OpenCL > Vulkan

    // Create temporary instances to check availability
    CUDAProcessor cudaProc;
    if (cudaProc.IsAvailable()) {
        return IGPUProcessor::Backend::CUDA;
    }

    OpenCLProcessor openclProc;
    if (openclProc.IsAvailable()) {
        return IGPUProcessor::Backend::OPENCL;
    }

    VulkanProcessor vulkanProc;
    if (vulkanProc.IsAvailable()) {
        return IGPUProcessor::Backend::VULKAN;
    }

    // If no GPU is available, return VULKAN as fallback which may use CPU
    return IGPUProcessor::Backend::VULKAN;  // Default fallback
}

std::vector<IGPUProcessor::Backend> GPUProcessorFactory::GetSupportedBackends() {
    std::vector<IGPUProcessor::Backend> backends;

    // Check each backend for actual availability
    CUDAProcessor cudaProc;
    if (cudaProc.IsAvailable()) {
        backends.push_back(IGPUProcessor::Backend::CUDA);
    }

    OpenCLProcessor openclProc;
    if (openclProc.IsAvailable()) {
        backends.push_back(IGPUProcessor::Backend::OPENCL);
    }

    VulkanProcessor vulkanProc;
    if (vulkanProc.IsAvailable()) {
        backends.push_back(IGPUProcessor::Backend::VULKAN);
    }

    return backends;
}
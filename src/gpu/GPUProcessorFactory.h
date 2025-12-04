#ifndef GPU_PROCESSOR_FACTORY_H
#define GPU_PROCESSOR_FACTORY_H

#include "IGPUProcessor.h"
#include <memory>
#include <vector>

/**
 * @brief Factory class for creating GPU processors based on backend type
 */
class GPUProcessorFactory {
public:
    /**
     * @brief Create a new GPU processor instance with specified backend
     * @param backend The GPU backend to use (CUDA, OpenCL, Vulkan)
     * @return Unique pointer to a new IGPUProcessor instance or nullptr if not supported
     */
    static std::unique_ptr<IGPUProcessor> CreateProcessor(IGPUProcessor::Backend backend);

    /**
     * @brief Detect and select the best available GPU backend automatically
     * @return Backend type that was selected, or UNKNOWN if no GPUs found
     */
    static IGPUProcessor::Backend AutoDetectBestGPU();

    /**
     * @brief Get a list of supported GPU backends on this system
     * @return Vector with all supported backend types
     */
    static std::vector<IGPUProcessor::Backend> GetSupportedBackends();
};

#endif // GPU_PROCESSOR_FACTORY_H
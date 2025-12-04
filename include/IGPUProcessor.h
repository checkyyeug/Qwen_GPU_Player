#ifndef I_GPU_PROCESSOR_H
#define I_GPU_PROCESSOR_H

#include <string>
#include <memory>

/**
 * @brief GPU Processor interface for audio processing acceleration
 */
class IGPUProcessor {
public:
    /**
     * @brief Enum to specify different GPU backends
     */
    enum class Backend {
        CUDA,
        OPENCL,
        VULKAN
    };
    
    /**
     * @brief Constructor
     */
    IGPUProcessor() = default;
    
    /**
     * @brief Destructor
     */
    virtual ~IGPUProcessor() = default;
    
    /**
     * @brief Initialize the GPU processor with specified backend
     * @param backend The GPU backend to use (CUDA, OpenCL, Vulkan)
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Initialize(Backend backend) = 0;
    
    /**
     * @brief Process audio data using GPU acceleration
     * @param inputBuffer Input buffer containing raw audio samples
     * @param outputBuffer Output buffer for processed audio samples
     * @param bufferSize Size of the buffers in bytes
     * @return true if processing was successful, false otherwise
     */
    virtual bool ProcessAudio(const float* inputBuffer, 
                           float* outputBuffer,
                           size_t bufferSize) = 0;
    
    /**
     * @brief Get GPU information string
     * @return String with detailed GPU information
     */
    virtual std::string GetGPUInfo() const = 0;
    
    /**
     * @brief Check if the processor is available and functional
     * @return true if available, false otherwise
     */
    virtual bool IsAvailable() const = 0;
};

#endif // I_GPU_PROCESSOR_H
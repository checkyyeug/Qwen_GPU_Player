#ifndef I_GPU_PROCESSOR_H
#define I_GPU_PROCESSOR_H

#include <string>
#include <memory>

/**
 * @brief Audio processing parameters structure for advanced GPU processing
 */
struct AudioProcessingParams {
    // EQ parameters
    double lowFreq;        // Low frequency point
    double lowGain;        // Low frequency gain adjustment
    double lowQ;           // Low frequency Q value
    double highFreq;       // High frequency point
    double highGain;       // High frequency gain adjustment
    double highQ;          // High frequency Q value

    // Format parameters
    int targetSampleRate;  // Target sample rate
    int targetBitrate;    // Target bitrate
    int bitDepth;         // Target bit depth

    // Performance parameters
    int quality;          // Quality level (0-10)
    bool enableFilters;   // Whether to enable filters
    bool enableResampling; // Whether to enable sample rate conversion
    bool enableBitrateConversion; // Whether to enable bitrate conversion
};

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
     * @brief Convert audio sample rate using GPU acceleration
     * @param inputBuffer Input audio buffer
     * @param inputSampleRate Sample rate of input buffer
     * @param outputBuffer Output audio buffer (should be pre-allocated)
     * @param outputSampleRate Target sample rate for output
     * @param inputSampleCount Number of samples in input buffer
     * @param outputSampleCount Number of samples in output buffer (calculated)
     * @return true if conversion was successful, false otherwise
     */
    virtual bool ConvertSampleRate(const float* inputBuffer,
                                   int inputSampleRate,
                                   float* outputBuffer,
                                   int outputSampleRate,
                                   size_t inputSampleCount,
                                   size_t& outputSampleCount) {
        // Default implementation returns false - needs to be overridden
        return false;
    }

    /**
     * @brief Convert audio bitrate using GPU acceleration
     * @param inputBuffer Input audio buffer
     * @param inputBitrate Input bitrate in kbps
     * @param outputBuffer Output audio buffer (should be pre-allocated)
     * @param targetBitrate Target bitrate in kbps
     * @param bufferSize Size of input buffer in bytes
     * @return true if conversion was successful, false otherwise
     */
    virtual bool ConvertBitrate(const float* inputBuffer,
                               int inputBitrate,
                               float* outputBuffer,
                               int targetBitrate,
                               size_t bufferSize) = 0;

    /**
     * @brief Process audio with specified parameters using GPU acceleration
     * @param inputBuffer Input audio buffer
     * @param outputBuffer Output audio buffer (should be pre-allocated)
     * @param bufferSize Size of buffers in samples
     * @param parameters Processing parameters (EQ, filters, etc.)
     * @return true if processing was successful, false otherwise
     */
    virtual bool ProcessAudioWithParams(const float* inputBuffer,
                                       float* outputBuffer,
                                       size_t bufferSize,
                                       const struct AudioProcessingParams& parameters) {
        // Default implementation falls back to basic ProcessAudio
        return ProcessAudio(inputBuffer, outputBuffer, bufferSize);
    }

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
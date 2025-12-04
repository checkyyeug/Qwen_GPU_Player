#ifndef I_AUDIO_DECODER_H
#define I_AUDIO_DECODER_H

#include <string>
#include <memory>

/**
 * @brief Audio decoder interface for handling different audio formats
 */
class IAudioDecoder {
public:
    /**
     * @brief Constructor
     */
    IAudioDecoder() = default;
    
    /**
     * @brief Destructor
     */
    virtual ~IAudioDecoder() = default;
    
    /**
     * @brief Check if this decoder can handle a specific file format
     * @param filePath Path to the audio file
     * @return true if supported, false otherwise
     */
    virtual bool CanHandle(const std::string& filePath) const = 0;
    
    /**
     * @brief Open and initialize decoding for a given file path
     * @param filePath Path to the audio file
     * @return true if successful, false otherwise
     */
    virtual bool OpenFile(const std::string& filePath) = 0;
    
    /**
     * @brief Read next chunk of decoded audio data
     * @param buffer Output buffer for raw audio samples
     * @param bufferSize Size of the output buffer in bytes
     * @return Number of bytes read, or -1 on error
     */
    virtual int ReadNextChunk(float* buffer, size_t bufferSize) = 0;
    
    /**
     * @brief Get information about the decoded file (sample rate, channels, etc.)
     * @param filePath Path to the audio file
     * @return String with detailed file information
     */
    virtual std::string GetFileInfo(const std::string& filePath) const = 0;
    
    /**
     * @brief Close and cleanup decoding resources
     */
    virtual void CloseFile() = 0;
};

#endif // I_AUDIO_DECODER_H
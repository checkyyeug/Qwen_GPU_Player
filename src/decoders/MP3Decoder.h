#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include "IAudioDecoder.h"
#include <string>

/**
 * @brief MP3 decoder implementation for audio processing
 */
class MP3Decoder : public IAudioDecoder {
public:
    /**
     * @brief Constructor
     */
    MP3Decoder();
    
    /**
     * @brief Destructor
     */
    ~MP3Decoder() override;
    
    /**
     * @brief Check if this decoder can handle a specific file format
     * @param filePath Path to the audio file
     * @return true for MP3 files, false otherwise
     */
    bool CanHandle(const std::string& filePath) const override;
    
    /**
     * @brief Open and initialize decoding for an MP3 file
     * @param filePath Path to the audio file
     * @return true if successful, false otherwise
     */
    bool OpenFile(const std::string& filePath) override;
    
    /**
     * @brief Read next chunk of decoded audio data from MP3 file
     * @param buffer Output buffer for raw audio samples
     * @param bufferSize Size of the output buffer in bytes
     * @return Number of bytes read, or -1 on error
     */
    int ReadNextChunk(float* buffer, size_t bufferSize) override;
    
    /**
     * @brief Get information about an MP3 file
     * @param filePath Path to the audio file
     * @return String with detailed file information
     */
    std::string GetFileInfo(const std::string& filePath) const override;
    
    /**
     * @brief Close and cleanup decoding resources for MP3 files
     */
    void CloseFile() override;

private:
    // Private implementation details
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // MP3_DECODER_H
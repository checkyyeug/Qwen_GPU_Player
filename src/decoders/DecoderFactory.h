#ifndef DECODER_FACTORY_H
#define DECODER_FACTORY_H

#include <memory>
#include <string>
#include "IAudioDecoder.h"

/**
 * @brief Factory class for creating audio decoders based on file format
 */
class DecoderFactory {
public:
    /**
     * @brief Create an appropriate decoder for the given file path
     * @param filePath Path to the audio file
     * @return Unique pointer to a new IAudioDecoder instance, or nullptr if unsupported
     */
    static std::unique_ptr<IAudioDecoder> CreateDecoder(const std::string& filePath);
    
    /**
     * @brief Register a decoder with the factory
     * @param format File extension/format that this decoder handles
     * @param decoderFactory Function to create decoder instances
     */
    static void RegisterDecoder(const std::string& format, 
                                std::unique_ptr<IAudioDecoder>(*decoderFactory)(const std::string&));
    
private:
    // Private implementation details
    class Impl;
    static std::unique_ptr<Impl> pImpl;
};

#endif // DECODER_FACTORY_H
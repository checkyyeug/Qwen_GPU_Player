#include "DecoderFactory.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>  // Added for std::transform

// Implementation of DecoderFactory

class DecoderFactory::Impl {
public:
    std::unordered_map<std::string, 
                       std::unique_ptr<IAudioDecoder>(*)(const std::string&)> decoders;
};

std::unique_ptr<DecoderFactory::Impl> DecoderFactory::pImpl = std::make_unique<Impl>();

std::unique_ptr<IAudioDecoder> DecoderFactory::CreateDecoder(const std::string& filePath) {
    // Extract file extension
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return nullptr;
    }
    
    std::string format = filePath.substr(dotPos + 1);
    // Convert to lowercase for case-insensitive matching
    std::transform(format.begin(), format.end(), format.begin(), 
                  [](unsigned char c){ return std::tolower(c); });
    
    // Check if we have a decoder registered for this format
    auto it = pImpl->decoders.find(format);
    if (it != pImpl->decoders.end()) {
        return (it->second)(filePath);
    }
    
    // Try to find a generic decoder that can handle the file type
    std::cout << "No specific decoder found for format: " << format 
              << ", trying generic decoder\n";
    return nullptr;
}

void DecoderFactory::RegisterDecoder(const std::string& format, 
                                 std::unique_ptr<IAudioDecoder>(*decoderFactory)(const std::string&)) {
    pImpl->decoders[format] = decoderFactory;
}
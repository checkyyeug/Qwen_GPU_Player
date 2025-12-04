#include "IAudioDecoder.h"
#include "DecoderFactory.h"
#include <iostream>

int main() {
    std::cout << "=== Decoder Factory Test ===\n";
    
    // Create a decoder for an MP3 file (this is just to demonstrate the interface)
    std::string mp3FilePath = "test.mp3";
    
    // Try to create a decoder
    auto decoder = DecoderFactory::CreateDecoder(mp3FilePath);
    
    if (!decoder) {
        std::cout << "No decoder found for: " << mp3FilePath << "\n";
        return 1;
    }
    
    // Check if it can handle the file (this would be implemented in actual decoders)
    bool canHandle = decoder->CanHandle(mp3FilePath);
    std::string fileInfo = decoder->GetFileInfo(mp3FilePath);
    
    std::cout << "Decoder test completed\n";
    std::cout << "Can Handle: " << (canHandle ? "true" : "false") << "\n";
    std::cout << "File Info:\n" << fileInfo << "\n";
    
    return 0;
}
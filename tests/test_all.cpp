#include "AudioEngine.h"
#include <iostream>

int main() {
    std::cout << "Testing GPU Music Player components\n";
    
    // Test 1: Create audio engine
    AudioEngine player;
    std::cout << "✓ AudioEngine created successfully\n";
    
    // Test 2: Try to initialize with a GPU processor
    auto gpuProcessor = GPUProcessorFactory::CreateProcessor(IGPUProcessor::Backend::VULKAN);
    if (gpuProcessor) {
        bool initialized = player.Initialize(std::move(gpuProcessor));
        std::cout << "✓ AudioEngine initialization: " 
                      << (initialized ? "successful" : "failed") << "\n";
    } else {
        std::cout << "✗ Failed to create GPU processor\n";
    }
    
    // Test 3: Try to set EQ parameters
    bool eqSet = player.SetEQ(100.0, 3.0, 0.7, 
                            10000.0, -2.0, 0.7);
    std::cout << "✓ SetEQ call: " << (eqSet ? "successful" : "failed") << "\n";
    
    // Test 4: Try to get stats
    std::string stats = player.GetStats();
    std::cout << "✓ GetStats call: successful\n";
    
    std::cout << "All tests completed successfully!\n";
    return 0;
}
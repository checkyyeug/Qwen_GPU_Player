#include "AudioEngine.h"
#include <iostream>

int main() {
    // Example usage of the GPU Music Player
    
    std::cout << "=== Basic Usage Examples ===\n\n";
    
    // 1. Initialize with a GPU processor (Vulkan for cross-platform compatibility)
    AudioEngine player;
    
    auto gpuProcessor = GPUProcessorFactory::CreateProcessor(IGPUProcessor::Backend::VULKAN);
    if (!player.Initialize(std::move(gpuProcessor))) {
        std::cout << "Failed to initialize audio engine\n";
        return 1;
    }
    
    // 2. Load and play an audio file
    std::string filePath = "music.flac";
    player.LoadFile(filePath);
    
    // 3. Set EQ parameters (100Hz +3dB, 10kHz -2dB)
    player.SetEQ(100.0, 3.0, 0.7, 10000.0, -2.0, 0.7);
    
    // 4. Start playback
    player.Play();
    
    // 5. Seek to a specific position (60 seconds)
    player.Seek(60.0);
    
    // 6. Pause/resume playback
    player.Pause();
    player.Pause(); // Resume
    
    // 7. Get performance statistics including GPU info
    std::string stats = player.GetStats();
    std::cout << "Performance Stats:\n" << stats << "\n";
    
    // 8. Stop playback
    player.Stop();
    
    return 0;
}
#include "AudioEngine.h"
#include "CommandLineInterface.h"  // Fixed include path
#include "IGPUProcessor.h"          // Include GPU processor interface
#include "gpu/GPUProcessorFactory.h" // Include GPU processor factory
#include <iostream>
#include <string>
#include <memory>                   // Include memory for std::move
#include <thread>                   // Include thread for sleep operations
#include <chrono>                   // Include chrono for time operations

int main(int argc, char* argv[]) {
    std::cout << "GPU Music Player v1.0\n";

    // Create an instance of the audio engine
    AudioEngine player;

    // Get all supported GPU backends
    auto supportedBackends = GPUProcessorFactory::GetSupportedBackends();

    std::cout << "Detected GPU backends: ";
    if (supportedBackends.empty()) {
        std::cout << "None\n";
    } else {
        for (size_t i = 0; i < supportedBackends.size(); i++) {
            switch(supportedBackends[i]) {
                case IGPUProcessor::Backend::CUDA:
                    std::cout << "CUDA ";
                    break;
                case IGPUProcessor::Backend::OPENCL:
                    std::cout << "OpenCL ";
                    break;
                case IGPUProcessor::Backend::VULKAN:
                    std::cout << "Vulkan ";
                    break;
            }
            if (i < supportedBackends.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }

    // Auto-detect the best GPU processor available
    auto bestBackend = GPUProcessorFactory::AutoDetectBestGPU();

    std::cout << "Auto-selected GPU backend: ";
    switch(bestBackend) {
        case IGPUProcessor::Backend::CUDA:
            std::cout << "CUDA (NVIDIA GPU)\n";
            break;
        case IGPUProcessor::Backend::OPENCL:
            std::cout << "OpenCL (AMD/Intel GPU)\n";
            break;
        case IGPUProcessor::Backend::VULKAN:
            std::cout << "Vulkan (Universal GPU API)\n";
            break;
    }

    auto gpuProcessor = GPUProcessorFactory::CreateProcessor(bestBackend);
    if (!player.Initialize(std::move(gpuProcessor))) {
        std::cout << "Failed to initialize audio engine\n";
        return 1;
    }

    // If a file path is provided as command line argument, play it
    if (argc > 1) {
        std::string filePath = argv[1];
        std::cout << "Loading file: " << filePath << "\n";

        if (player.LoadFile(filePath)) {
            player.Play();

            // For command-line usage, we should return control to user immediately
            // but the playback continues in background
            std::cout << "Playback started in background. Switching to interactive mode.\n";

            // Don't wait - allow playback to continue in background while user can enter commands
            // The playback thread will continue running in the background
        } else {
            std::cout << "Failed to load file\n";
        }
    }

    // Interactive mode - process commands from user input
    std::cout << "Interactive mode started. Type 'help' for available commands.\n";

    CommandLineInterface cli(player);
    std::string command;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, command)) break;

        // Exit on quit or exit
        if (command == "quit" || command == "exit") {
            break;
        }

        cli.ProcessCommand(command);
    }

    std::cout << "Exiting GPU Music Player...\n";
    return 0;
}
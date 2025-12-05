#include "CommandLineInterface.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

// Implementation of CommandLineInterface

CommandLineInterface::CommandLineInterface(AudioEngine& engine) : engine(engine) {}

CommandLineInterface::~CommandLineInterface() = default;

bool CommandLineInterface::ProcessCommand(const std::string& command) {
    // Split the command into arguments
    auto args = SplitCommand(command);

    if (args.empty()) {
        return false;
    }

    // Convert first argument to lowercase for case-insensitive matching
    std::transform(args[0].begin(), args[0].end(), args[0].begin(),
                    [](unsigned char c){ return std::tolower(c); });

    return ExecuteCommand(args);
}

bool CommandLineInterface::ExecuteCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return false;
    }

    const auto& command = args[0];

    // Handle different commands
    if (command == "play") {
        if (args.size() < 2) {
            std::cout << "Usage: play <file_path>\n";
            return false;
        }
        return HandlePlay(args[1]);
    }
    else if (command == "load") {
        if (args.size() < 2) {
            std::cout << "Usage: load <file_path>\n";
            return false;
        }
        return HandleLoad(args[1]);
    }
    else if (command == "pause" || command == "toggle") {
        return HandlePause();
    }
    else if (command == "stop") {
        return HandleStop();
    }
    else if (command == "seek") {
        if (args.size() < 2) {
            std::cout << "Usage: seek <seconds>\n";
            return false;
        }

        try {
            double seconds = std::stod(args[1]);
            return HandleSeek(seconds);
        } catch (...) {
            std::cout << "Invalid time value\n";
            return false;
        }
    }
    else if (command == "eq") {
        if (args.size() < 7) { // Changed from 6 to 7 since we have 7 arguments
            std::cout << "Usage: eq <f1> <g1> <q1> <f2> <g2> <q2>\n";
            return false;
        }

        try {
            double f1 = std::stod(args[1]);
            double g1 = std::stod(args[2]);
            double q1 = std::stod(args[3]);
            double f2 = std::stod(args[4]);
            double g2 = std::stod(args[5]);
            double q2 = std::stod(args[6]);

            return HandleEQ(f1, g1, q1, f2, g2, q2);
        } catch (...) {
            std::cout << "Invalid EQ parameter values\n";
            return false;
        }
    }
    else if (command == "bitrate") {
        if (args.size() < 2) {
            std::cout << "Usage: bitrate <target_kbps>\n";
            return false;
        }

        try {
            int targetBitrate = std::stoi(args[1]);
            return HandleBitrate(targetBitrate);
        } catch (...) {
            std::cout << "Invalid bitrate value\n";
            return false;
        }
    }
    else if (command == "convert") {
        if (args.size() < 3) {
            std::cout << "Usage: convert <input_file> <output_file> [target_bitrate]\n";
            std::cout << "  If bitrate is not provided, uses original file's bitrate\n";
            return false;
        }

        int targetBitrate = 0;  // Default to use original bitrate
        if (args.size() >= 4) {
            try {
                targetBitrate = std::stoi(args[3]);
            } catch (...) {
                std::cout << "Invalid bitrate value\n";
                return false;
            }
        }

        return HandleConvert(args[1], args[2], targetBitrate);
    }
    else if (command == "save") {
        if (args.size() < 2) {
            std::cout << "Usage: save <output_file>\n";
            return false;
        }

        return HandleSave(args[1]);
    }
    else if (command == "stats") {
        return HandleStats();
    }
    else if (command == "quit" || command == "exit") {
        return HandleQuit();
    }

    else if (command == "help") {
        std::cout << "Available commands:\n"
                  << "  play <file_path> - Play an audio file\n"
                  << "  load <file_path> - Load an audio file without playing\n"
                  << "  pause/toggle - Pause or resume playback\n"
                  << "  stop - Stop playback\n"
                  << "  seek <seconds> - Seek to a specific position\n"
                  << "  eq <f1> <g1> <q1> <f2> <g2> <q2> - Set EQ parameters\n"
                  << "  bitrate <kbps> - Set target bitrate for GPU conversion\n"
                  << "  convert <input> <output> [bitrate] - Convert file with GPU acceleration\n"
                  << "  save <file_path> - Save processed audio to file\n"
                  << "  stats - Show performance statistics\n"
                  << "  help - Show this help message\n"
                  << "  quit/exit - Exit the player\n";
        return true;
    }

    // Unknown command
    std::cout << "Unknown command: " << command << "\n";
    std::cout << "Available commands:\n"
              << "  play <file_path> - Play an audio file\n"
              << "  pause/toggle - Pause or resume playback\n"
              << "  stop - Stop playback\n"
              << "  seek <seconds> - Seek to a specific position\n"
              << "  eq <f1> <g1> <q1> <f2> <g2> <q2> - Set EQ parameters\n"
              << "  stats - Show performance statistics\n"
              << "  help - Show this help message\n"
              << "  quit/exit - Exit the player\n";
    return false;
}

std::vector<std::string> CommandLineInterface::SplitCommand(const std::string& command) {
    std::vector<std::string> args;
    std::stringstream ss(command);
    std::string arg;

    while (ss >> arg) {
        args.push_back(arg);
    }

    return args;
}

bool CommandLineInterface::HandlePlay(const std::string& filePath) {
    std::cout << "Loading file: " << filePath << "\n";

    // Call the engine's LoadFile method
    if (!engine.LoadFile(filePath)) {
        return false;
    }

    std::cout << "Starting playback of " << filePath << "\n";

    // Call the engine's Play method to actually play the audio
    return engine.Play();
}

bool CommandLineInterface::HandlePause() {
    std::cout << "Toggling pause\n";
    // In a real implementation, we would call engine.Pause()
    return true;
}

bool CommandLineInterface::HandleStop() {
    std::cout << "Stopping playback\n";
    // In a real implementation, we would call engine.Stop()
    return true;
}

bool CommandLineInterface::HandleSeek(double seconds) {
    std::cout << "Seeking to: " << seconds << " seconds\n";
    // In a real implementation, we would call engine.Seek(seconds)
    return true;
}

bool CommandLineInterface::HandleEQ(double freq1, double gain1, double q1,
                                        double freq2, double gain2, double q2) {
    std::cout << "Setting EQ parameters:\n"
              << "  Low: F=" << freq1 << ", G=" << gain1 << ", Q=" << q1 << "\n"
              << "  High: F=" << freq2 << ", G=" << gain2 << ", Q=" << q2 << "\n";
    // In a real implementation, we would call engine.SetEQ(freq1, gain1, q1, freq2, gain2, q2)
    return true;
}

bool CommandLineInterface::HandleStats() {
    std::cout << "Performance Statistics:\n";
    // In a real implementation, we would call engine.GetStats()
    std::string stats = engine.GetStats();
    std::cout << stats;
    return true;
}

bool CommandLineInterface::HandleBitrate(int targetBitrate) {
    std::cout << "Setting target bitrate to " << targetBitrate << " kbps using GPU acceleration\n";

    // In a real implementation, we would call engine.SetTargetBitrate(targetBitrate)
    bool success = engine.SetTargetBitrate(targetBitrate);

    if (success) {
        std::cout << "Bitrate conversion successfully applied using GPU\n";
    } else {
        std::cout << "Bitrate conversion failed. Using original audio quality.\n";
    }

    return success;
}

bool CommandLineInterface::HandleSave(const std::string& targetPath) {
    std::cout << "Saving processed audio to: " << targetPath << "\n";

    // In a real implementation, we would call engine.SaveFile(targetPath)
    bool success = engine.SaveFile(targetPath);

    if (success) {
        std::cout << "Successfully saved audio to: " << targetPath << "\n";
    } else {
        std::cout << "Failed to save audio to: " << targetPath << "\n";
    }

    return success;
}

bool CommandLineInterface::HandleConvert(const std::string& inputPath, const std::string& outputPath, int targetBitrate) {
    std::cout << "Converting: " << inputPath << " -> " << outputPath << "\n";

    // First, load the input file
    if (!engine.LoadFile(inputPath)) {
        std::cout << "Failed to load input file: " << inputPath << "\n";
        return false;
    }

    // If a target bitrate is specified, apply it
    if (targetBitrate > 0) {
        std::cout << "Applying target bitrate: " << targetBitrate << " kbps\n";
        if (!engine.SetTargetBitrate(targetBitrate)) {
            std::cout << "Warning: Could not apply target bitrate, using original\n";
        }
    }

    // Save the processed audio to the output file
    bool success = engine.SaveFile(outputPath);

    if (success) {
        std::cout << "File converted successfully: " << inputPath << " -> " << outputPath << "\n";
    } else {
        std::cout << "Conversion failed: " << inputPath << " -> " << outputPath << "\n";
    }

    return success;
}

bool CommandLineInterface::HandleLoad(const std::string& filePath) {
    std::cout << "Loading file: " << filePath << "\n";

    // Just call the engine's LoadFile method - this will load the audio data
    // without automatically starting playback
    bool success = engine.LoadFile(filePath);

    if (success) {
        std::cout << "File loaded successfully: " << filePath << "\n";
        std::cout << "Use 'play' command to start playback or other commands for processing\n";
    } else {
        std::cout << "Failed to load file: " << filePath << "\n";
    }

    return success;
}

bool CommandLineInterface::HandleQuit() {
    std::cout << "Exiting GPU Music Player...\n";
    return true;
}
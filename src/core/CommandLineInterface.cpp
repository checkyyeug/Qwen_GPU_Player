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
    else if (command == "stats") {
        return HandleStats();
    }
    else if (command == "quit" || command == "exit") {
        return HandleQuit();
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
    std::cout << "Playing: " << filePath << "\n";
    // In a real implementation, we would call engine.LoadFile(filePath)
    // and then engine.Play()
    return true;
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

bool CommandLineInterface::HandleQuit() {
    std::cout << "Exiting GPU Music Player...\n";
    return true;
}
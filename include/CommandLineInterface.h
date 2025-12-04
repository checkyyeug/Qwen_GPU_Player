#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

#include "AudioEngine.h"
#include <string>
#include <vector>

/**
 * @brief Command-line interface for the GPU Music Player
 */
class CommandLineInterface {
public:
    /**
     * @brief Constructor
     */
    CommandLineInterface(AudioEngine& engine);
    
    /**
     * @brief Destructor
     */
    ~CommandLineInterface();
    
    /**
     * @brief Process a command from user input
     * @param command Command string to process
     * @return true if command was processed successfully, false otherwise
     */
    bool ProcessCommand(const std::string& command);
    
    /**
     * @brief Parse and execute a command with arguments
     * @param args Vector of command arguments
     * @return true if successful, false otherwise
     */
    bool ExecuteCommand(const std::vector<std::string>& args);
    
private:
    // Reference to the audio engine
    AudioEngine& engine;
    
    /**
     * @brief Handle play command with file path argument
     * @param filePath Path to the audio file to play
     * @return true if successful, false otherwise
     */
    bool HandlePlay(const std::string& filePath);

    /**
     * @brief Handle load command with file path argument
     * @param filePath Path to the audio file to load
     * @return true if successful, false otherwise
     */
    bool HandleLoad(const std::string& filePath);

    /**
     * @brief Handle pause command
     * @return true if successful, false otherwise
     */
    bool HandlePause();
    
    /**
     * @brief Handle stop command
     * @return true if successful, false otherwise
     */
    bool HandleStop();
    
    /**
     * @brief Handle seek command with seconds argument
     * @param seconds Position to seek to in seconds
     * @return true if successful, false otherwise
     */
    bool HandleSeek(double seconds);
    
    /**
     * @brief Handle EQ command with parameters
     * @param freq1 Low frequency point
     * @param gain1 Low frequency gain adjustment
     * @param q1 Low frequency Q value
     * @param freq2 High frequency point
     * @param gain2 High frequency gain adjustment
     * @param q2 High frequency Q value
     * @return true if successful, false otherwise
     */
    bool HandleEQ(double freq1, double gain1, double q1,
                   double freq2, double gain2, double q2);
    
    /**
     * @brief Handle stats command to show performance information
     * @return true if successful, false otherwise
     */
    bool HandleStats();
    
    /**
     * @brief Handle quit/exit command
     * @return true if successful, false otherwise
     */
    bool HandleQuit();

    /**
     * @brief Handle save command to save processed audio to file
     * @param targetPath Target path for the output file
     * @return true if successful, false otherwise
     */
    bool HandleSave(const std::string& targetPath);

    /**
     * @brief Handle convert command to convert audio with GPU acceleration
     * @param inputPath Input file path
     * @param outputPath Output file path
     * @param targetBitrate Target bitrate for conversion (0 to use original)
     * @return true if successful, false otherwise
     */
    bool HandleConvert(const std::string& inputPath, const std::string& outputPath, int targetBitrate);

    /**
     * @brief Handle bitrate command to set target bitrate
     * @param targetBitrate Target bitrate in kbps
     * @return true if successful, false otherwise
     */
    bool HandleBitrate(int targetBitrate);

    // Helper functions for parsing commands
    std::vector<std::string> SplitCommand(const std::string& command);
};

#endif // COMMAND_LINE_INTERFACE_H
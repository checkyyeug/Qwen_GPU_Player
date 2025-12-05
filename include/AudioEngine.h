#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <string>
#include <memory>   // Added for std::unique_ptr
#include <vector>   // Added for std::vector
#include <mutex>    // Added for std::mutex and std::atomic
#include <atomic>   // Added for std::atomic

// Forward declaration for GPU interface
class IGPUProcessor;

// Note: These interfaces are placeholders for the original design
// The actual implementation uses only the IGPUProcessor interface
// class IAudioDecoder;
// class IAudioDevice;

/**
 * @brief Main audio engine interface that coordinates all components
 */
class AudioEngine {
public:
    /**
     * @brief Constructor
     */
    AudioEngine();

    /**
     * @brief Destructor
     */
    virtual ~AudioEngine();

    /**
     * @brief Initialize the audio engine with specified parameters
     * @param gpuProcessor The GPU processor to use for acceleration
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize(std::unique_ptr<IGPUProcessor> gpuProcessor);

    /**
     * @brief Load an audio file for playback
     * @param filePath Path to the audio file
     * @return true if loading was successful, false otherwise
     */
    bool LoadFile(const std::string& filePath);

    /**
     * @brief Start playing the loaded audio file
     * @return true if playback started successfully, false otherwise
     */
    bool Play();

    /**
     * @brief Pause or resume playback
     * @return true if operation was successful, false otherwise
     */
    bool Pause();

    /**
     * @brief Stop playback and reset the engine
     * @return true if operation was successful, false otherwise
     */
    bool Stop();

    /**
     * @brief Seek to a specific position in the audio file (in seconds)
     * @param seconds Position to seek to
     * @return true if seeking was successful, false otherwise
     */
    bool Seek(double seconds);

    /**
     * @brief Set EQ parameters for audio processing
     * @param freq1 Low frequency point
     * @param gain1 Low frequency gain adjustment
     * @param q1 Low frequency Q value
     * @param freq2 High frequency point
     * @param gain2 High frequency gain adjustment
     * @param q2 High frequency Q value
     * @return true if parameters were set successfully, false otherwise
     */
    bool SetEQ(double freq1, double gain1, double q1,
                double freq2, double gain2, double q2);


    /**
     * @brief Get performance statistics including GPU information
     * @return String with performance stats
     */
    std::string GetStats();

    /**
     * @brief Check if an audio file is currently loaded
     * @return true if a file is loaded and ready to play, false otherwise
     */
    bool IsFileLoaded() const;

    /**
     * @brief Get current playback state
     * @return true if currently playing audio, false otherwise
     */
    bool IsPlaying() const;

    /**
     * @brief Get current pause state
     * @return true if paused, false otherwise
     */
    bool IsPaused() const;

    /**
     * @brief Set processing parameters for audio engine
     * @param params Processing parameters to apply
     */
    void SetProcessingParams(const struct ProcessingParams& params);

    /**
     * @brief Set target bitrate for audio processing (with GPU acceleration)
     * @param targetBitrate Target bitrate in kbps
     * @return true if bitrate conversion was successful, false otherwise
     */
    bool SetTargetBitrate(int targetBitrate);

    /**
     * @brief Save processed audio to a file
     * @param filePath Path where the processed audio file should be saved
     * @return true if save was successful, false otherwise
     */
    bool SaveFile(const std::string& filePath);

    /**
     * @brief Get the current playback status
     * @return Current playback state (Stopped, Paused, Playing)
     */
    enum class PlaybackState {
        Stopped,
        Playing,
        Paused
    };

    /**
     * @brief Get the current playback state
     * @return Current playback state
     */
    PlaybackState GetPlaybackState() const;

    /**
     * @brief Get the current playback position in seconds
     * @return Current playback time in seconds
     */
    double GetCurrentPosition() const;

private:
    // Private implementation details
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // AUDIO_ENGINE_H
#ifndef AUDIO_DEVICE_DRIVER_H
#define AUDIO_DEVICE_DRIVER_H

#include "IAudioDevice.h"
#include <string>

/**
 * @brief Audio device driver implementation for different platforms (ASIO, CoreAudio, ALSA)
 */
class AudioDeviceDriver : public IAudioDevice {
public:
    /**
     * @brief Constructor
     */
    AudioDeviceDriver();
    
    /**
     * @brief Destructor
     */
    ~AudioDeviceDriver() override;
    
    /**
     * @brief Initialize the audio device with specified parameters
     * @param outputType Type of audio output to use (ASIO, CoreAudio, ALSA)
     * @param deviceId ID of the audio device to use
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize(OutputType outputType, const std::string& deviceId) override;
    
    /**
     * @brief Start playing audio data through this device
     * @return true if playback started successfully, false otherwise
     */
    bool Play() override;
    
    /**
     * @brief Pause or resume audio playback
     * @return true if operation was successful, false otherwise
     */
    bool Pause() override;
    
    /**
     * @brief Stop audio playback and cleanup resources
     */
    void Stop() override;
    
    /**
     * @brief Write audio data to the device buffer
     * @param buffer Audio data to write
     * @param bufferSize Size of the audio data in bytes
     * @return Number of bytes written, or -1 on error
     */
    int Write(const float* buffer, size_t bufferSize) override;
    
    /**
     * @brief Get information about this audio device
     * @return String with detailed device information
     */
    std::string GetDeviceInfo() const override;
    
    /**
     * @brief Check if the device is available and functional
     * @return true if available, false otherwise
     */
    bool IsAvailable() const override;

private:
    // Private implementation details
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // AUDIO_DEVICE_DRIVER_H
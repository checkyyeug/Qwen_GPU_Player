#ifndef I_AUDIO_DEVICE_H
#define I_AUDIO_DEVICE_H

#include <string>
#include <memory>

/**
 * @brief Audio device interface for low-latency audio output
 */
class IAudioDevice {
public:
    /**
     * @brief Enum to specify different audio output types
     */
    enum class OutputType {
        ASIO,
        COREAUDIO,
        ALSA,
        DIRECTSOUND,
        WAVEOUT
    };
    
    /**
     * @brief Constructor
     */
    IAudioDevice() = default;
    
    /**
     * @brief Destructor
     */
    virtual ~IAudioDevice() = default;
    
    /**
     * @brief Initialize the audio device with specified parameters
     * @param outputType Type of audio output to use (ASIO, CoreAudio, ALSA)
     * @param deviceId ID of the audio device to use
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Initialize(OutputType outputType, const std::string& deviceId) = 0;
    
    /**
     * @brief Start playing audio data through this device
     * @return true if playback started successfully, false otherwise
     */
    virtual bool Play() = 0;
    
    /**
     * @brief Pause or resume audio playback
     * @return true if operation was successful, false otherwise
     */
    virtual bool Pause() = 0;
    
    /**
     * @brief Stop audio playback and cleanup resources
     */
    virtual void Stop() = 0;
    
    /**
     * @brief Write audio data to the device buffer
     * @param buffer Audio data to write
     * @param bufferSize Size of the audio data in bytes
     * @return Number of bytes written, or -1 on error
     */
    virtual int Write(const float* buffer, size_t bufferSize) = 0;
    
    /**
     * @brief Get information about this audio device
     * @return String with detailed device information
     */
    virtual std::string GetDeviceInfo() const = 0;
    
    /**
     * @brief Check if the device is available and functional
     * @return true if available, false otherwise
     */
    virtual bool IsAvailable() const = 0;
};

#endif // I_AUDIO_DEVICE_H
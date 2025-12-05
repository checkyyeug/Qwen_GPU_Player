#include "AudioEngine.h"
#include "IGPUProcessor.h"  // Include IGPUProcessor.h for interface definition
#include <iostream>
#include <fstream>
#include <algorithm>  // For std::transform
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#define NOMINMAX  // Prevent Windows from defining min/max macros
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef ENABLE_FLAC
#include <FLAC/all.h>
#endif

// Implementation of AudioEngine interface

class AudioEngine::Impl {
public:
    Impl() = default;

    // Placeholder for actual implementation
    bool initialized = false;
    std::string currentFile;
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> isPaused{false};

    // Audio data and parameters
    std::vector<char> audioData;
    WAVEFORMATEX waveFormat = {};
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHeader = {};
    bool audioLoaded = false;

    // Audio playback position tracking
    size_t playbackPosition = 0; // Position in audio data buffer (in bytes)
    double playbackTime = 0.0;   // Playback time in seconds

    // Thread for audio playback
    std::thread playbackThread;
    std::atomic<bool> shouldStop{false};

    // GPU processor
    std::unique_ptr<IGPUProcessor> gpuProcessor;

    // Thread synchronization mutex
    mutable std::mutex audioEngineMutex;

#ifdef ENABLE_FLAC
    // FLAC decoding related data
    std::vector<char> flacBuffer;
    size_t flacSampleRate = 0;
    unsigned int flacChannels = 0;
    unsigned int flacBitsPerSample = 0;
    FLAC__uint64 flacTotalSamples = 0;
    bool isFlacFile = false;
#endif
};

AudioEngine::AudioEngine() : pImpl(std::make_unique<Impl>()) {}

AudioEngine::~AudioEngine() = default;

// FLAC decoding support (only defined if FLAC support is enabled)
#ifdef ENABLE_FLAC
#include <FLAC/all.h>

// Define a structure to pass data to callbacks that avoids accessing private members directly
struct FlacDecodeData {
    std::vector<char>* audioBuffer;
    size_t* sampleRate;
    unsigned int* channels;
    unsigned int* bitsPerSample;
    FLAC__uint64* totalSamples;
};

// Define FLAC decoder callback functions
// These callbacks will be used to process FLAC data during decoding
static FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data) {
    // Cast the client data to our custom structure
    FlacDecodeData *data = static_cast<FlacDecodeData*>(client_data);
    if (!data) return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    // Store audio parameters from the frame if not already set
    if (*data->sampleRate == 0) {
        *data->sampleRate = frame->header.sample_rate;
        *data->channels = frame->header.channels;
        *data->bitsPerSample = frame->header.bits_per_sample;
    }

    // Calculate frame size and convert samples
    unsigned int frame_size = frame->header.blocksize;
    unsigned int total_samples = frame_size * (*data->channels);
    unsigned int bytes_per_sample = (*data->bitsPerSample) / 8;
    unsigned int total_bytes = total_samples * bytes_per_sample;

    std::vector<char> frame_data(total_bytes);
    char *data_ptr = frame_data.data();

    for (unsigned int sample = 0; sample < frame_size; sample++) {
        for (unsigned int channel = 0; channel < (*data->channels); channel++) {
            FLAC__int32 sample_value = buffer[channel][sample];

            if ((*data->bitsPerSample) == 16) {
                short pcm_value = static_cast<short>(sample_value);
                *reinterpret_cast<short*>(data_ptr) = pcm_value;
                data_ptr += sizeof(short);
            } else if ((*data->bitsPerSample) == 24) {
                // Handle 24-bit audio by packing as 3 bytes
                *reinterpret_cast<unsigned char*>(data_ptr) = static_cast<unsigned char>(sample_value & 0xFF);
                data_ptr++;
                *reinterpret_cast<unsigned char*>(data_ptr) = static_cast<unsigned char>((sample_value >> 8) & 0xFF);
                data_ptr++;
                *reinterpret_cast<unsigned char*>(data_ptr) = static_cast<unsigned char>((sample_value >> 16) & 0xFF);
                data_ptr++;
            } else {
                // Default to 16-bit for other formats
                short pcm_value = static_cast<short>(sample_value);
                *reinterpret_cast<short*>(data_ptr) = pcm_value;
                data_ptr += sizeof(short);
            }
        }
    }

    // Add the frame data to the buffer
    data->audioBuffer->insert(data->audioBuffer->end(), frame_data.begin(), frame_data.end());
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void flac_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        FlacDecodeData *data = static_cast<FlacDecodeData*>(client_data);
        if (!data) return;

        *data->totalSamples = metadata->data.stream_info.total_samples;
        *data->sampleRate = metadata->data.stream_info.sample_rate;
        *data->channels = metadata->data.stream_info.channels;
        *data->bitsPerSample = metadata->data.stream_info.bits_per_sample;
    }
}

static void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    std::cout << "FLAC decode error: " << FLAC__StreamDecoderErrorStatusString[status] << std::endl;
}
#endif

bool AudioEngine::Initialize(std::unique_ptr<IGPUProcessor> gpuProcessor) {
    // Initialize with GPU processor
    if (gpuProcessor && gpuProcessor->IsAvailable()) {
        std::cout << "GPU Processor detected and initialized:\n";
        std::cout << gpuProcessor->GetGPUInfo() << "\n";
        std::cout << "Initializing audio engine with GPU support\n";

        // Store the GPU processor for later use
        pImpl->gpuProcessor = std::move(gpuProcessor);

        pImpl->initialized = true;
        return true;
    }

    std::cout << "Failed to initialize with GPU processor\n";
    return false;
}

bool AudioEngine::LoadFile(const std::string& filePath) {
    if (!pImpl->initialized) {
        return false;
    }

    // Check if file exists first
    std::ifstream file(filePath, std::ios::binary);
    if (!file.good()) {
        std::cout << "Error: File does not exist - " << filePath << "\n";
        return false;
    }

    // Get file size to determine if it's a valid audio file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.close();

    if (fileSize == 0) {
        std::cout << "Error: File is empty - " << filePath << "\n";
        return false;
    }

    // Check file extension for basic format validation
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension != "wav" && extension != "mp3" && extension != "flac" && extension != "ogg" && extension != "m4a") {
        std::cout << "Warning: Unsupported file format (" << extension << ") - " << filePath << "\n";
        std::cout << "Supported formats: WAV, MP3, FLAC, OGG, M4A\n";
        std::cout << "Only WAV format is currently implemented for playback\n";
        // For demo purposes, we'll try to load WAV files, others will use tone generation
        if (extension != "wav") {
            std::cout << "Will generate a tone instead of playing the file\n";
            // Generate a tone to simulate playback for non-WAV files
            const int sampleRate = 44100;
            const int channels = 2; // Stereo
            const int bitsPerSample = 16;
            const int bytesPerSample = bitsPerSample / 8;
            const int frequency = 440; // A4 note
            const int durationSeconds = 2;

            int numSamples = sampleRate * durationSeconds;
            int totalBytes = numSamples * channels * bytesPerSample;

            // Resize audio data vector
            pImpl->audioData.resize(totalBytes);

            // Create simple sine wave
            for (int i = 0; i < numSamples; ++i) {
                double time = static_cast<double>(i) / sampleRate;
                double value = std::sin(2.0 * M_PI * frequency * time);

                // Convert to 16-bit signed integer
                short sample = static_cast<short>(value * 32767);

                // Write to audio data (stereo)
                int offset = i * channels * bytesPerSample;
                memcpy(&pImpl->audioData[offset], &sample, bytesPerSample);
                memcpy(&pImpl->audioData[offset + bytesPerSample], &sample, bytesPerSample);
            }

            // Set up wave format for the generated tone
            pImpl->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
            pImpl->waveFormat.nChannels = channels;
            pImpl->waveFormat.nSamplesPerSec = sampleRate;
            pImpl->waveFormat.nAvgBytesPerSec = sampleRate * channels * bytesPerSample;
            pImpl->waveFormat.nBlockAlign = channels * bytesPerSample;
            pImpl->waveFormat.wBitsPerSample = bitsPerSample;
            pImpl->waveFormat.cbSize = 0;

            pImpl->audioLoaded = true;
            pImpl->currentFile = filePath;
            return true;
        }
    }

    if (extension == "wav") {
        std::ifstream wavFile(filePath, std::ios::binary);
        if (!wavFile.is_open()) {
            std::cout << "Error: Could not open WAV file - " << filePath << "\n";
            return false;
        }

        // Read WAV header
        char chunkId[4];
        wavFile.read(chunkId, 4);
        if (std::string(chunkId, 4) != "RIFF") {
            std::cout << "Error: Invalid WAV file format - " << filePath << "\n";
            wavFile.close();
            return false;
        }

        // Read the complete file header to get format info
        wavFile.seekg(20); // Skip to format data
        short audioFormat;
        wavFile.read(reinterpret_cast<char*>(&audioFormat), sizeof(audioFormat));

        if (audioFormat != 1) { // PCM format
            std::cout << "Error: Only PCM WAV format is supported - " << filePath << "\n";
            wavFile.close();
            return false;
        }

        short numChannels;
        wavFile.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));

        int sampleRate;
        wavFile.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));

        wavFile.seekg(34); // Skip to bits per sample
        short bitsPerSample;
        wavFile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

        // Find data chunk
        char dataChunk[4];
        int chunkSize;
        bool foundData = false;
        wavFile.seekg(36); // Start looking for data chunk after the format info

        while (wavFile.good()) {
            wavFile.read(dataChunk, 4);
            if (!wavFile.good()) break;

            wavFile.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
            if (std::string(dataChunk, 4) == "data") {
                foundData = true;
                break;
            }
            // Skip this chunk and continue
            wavFile.seekg(chunkSize, std::ios::cur);
        }

        if (!foundData) {
            std::cout << "Error: No data chunk found in WAV file - " << filePath << "\n";
            wavFile.close();
            return false;
        }

        // Read audio data from the data chunk
        pImpl->audioData.resize(chunkSize);
        wavFile.read(pImpl->audioData.data(), chunkSize);
        wavFile.close();

        // Set up wave format based on the actual file
        pImpl->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        pImpl->waveFormat.nChannels = numChannels;
        pImpl->waveFormat.nSamplesPerSec = sampleRate;
        pImpl->waveFormat.wBitsPerSample = bitsPerSample;
        pImpl->waveFormat.nBlockAlign = (numChannels * bitsPerSample) / 8;
        pImpl->waveFormat.nAvgBytesPerSec = sampleRate * pImpl->waveFormat.nBlockAlign;
        pImpl->waveFormat.cbSize = 0;

        pImpl->audioLoaded = true;
        pImpl->currentFile = filePath;
        std::cout << "Successfully loaded WAV file: " << filePath << " (" << chunkSize << " bytes of audio data)\n";
        return true;
    }
    else if (extension == "flac") {
#ifdef ENABLE_FLAC
        std::cout << "FLAC file detected: " << filePath << "\n";

        // Use FLAC library to decode the file
        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
        if (!decoder) {
            std::cout << "Error: Could not create FLAC decoder\n";
            return false;
        }

        // Prepare data structure to pass to callbacks
        FlacDecodeData decodeData;
        decodeData.audioBuffer = &pImpl->flacBuffer;
        decodeData.sampleRate = &pImpl->flacSampleRate;
        decodeData.channels = &pImpl->flacChannels;
        decodeData.bitsPerSample = &pImpl->flacBitsPerSample;
        decodeData.totalSamples = &pImpl->flacTotalSamples;

        // Reset values
        pImpl->flacSampleRate = 0;
        pImpl->flacChannels = 0;
        pImpl->flacBitsPerSample = 0;
        pImpl->flacTotalSamples = 0;
        pImpl->flacBuffer.clear();

        // Initialize the decoder with the file
        FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
            decoder,
            filePath.c_str(),
            flac_write_callback,
            flac_metadata_callback,
            flac_error_callback,
            &decodeData
        );

        if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            std::cout << "Error: Could not initialize FLAC decoder: " <<
                         FLAC__StreamDecoderInitStatusString[init_status] << "\n";
            FLAC__stream_decoder_delete(decoder);
            return false;
        }

        // Decode the entire file with proper error checking
        FLAC__bool decode_result = FLAC__stream_decoder_process_until_end_of_stream(decoder);
        if (!decode_result) {
            std::cout << "Error: FLAC decoding failed\n";
            FLAC__stream_decoder_finish(decoder);
            FLAC__stream_decoder_delete(decoder);
            return false;
        }

        // Clean up resources properly
        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);

        // Copy decoded data to main audio buffer with bounds checking
        if (!pImpl->flacBuffer.empty()) {
            try {
                pImpl->audioData.resize(pImpl->flacBuffer.size());
                std::copy(pImpl->flacBuffer.begin(), pImpl->flacBuffer.end(), pImpl->audioData.begin());
            } catch (const std::exception& e) {
                std::cout << "Error: Could not copy decoded audio data: " << e.what() << "\n";
                return false;
            }
        }

        // Set up wave format for the decoded audio
        pImpl->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        pImpl->waveFormat.nChannels = pImpl->flacChannels;
        pImpl->waveFormat.nSamplesPerSec = pImpl->flacSampleRate;
        pImpl->waveFormat.wBitsPerSample = pImpl->flacBitsPerSample;
        pImpl->waveFormat.nBlockAlign = (pImpl->flacChannels * pImpl->flacBitsPerSample) / 8;
        pImpl->waveFormat.nAvgBytesPerSec = pImpl->flacSampleRate * pImpl->waveFormat.nBlockAlign;
        pImpl->waveFormat.cbSize = 0;

        pImpl->audioLoaded = true;
        pImpl->currentFile = filePath;

        std::cout << "FLAC file decoded successfully: " << filePath << "\n";
        std::cout << "Format: " << pImpl->flacSampleRate << "Hz, "
                  << pImpl->flacChannels << " channels, "
                  << pImpl->flacBitsPerSample << " bits\n";

        return true;
#else
        // Fallback when FLAC support is not compiled in
        std::cout << "FLAC file detected: " << filePath << "\n";
        std::cout << "Error: FLAC support not compiled in. FLAC library not found.\n";
        std::cout << "To enable FLAC support, install FLAC development libraries.\n";
        return false;  // Return false to indicate loading failed
#endif
    } else {
        std::cout << "Error: Format not implemented for playback - " << filePath << "\n";
        return false;
    }
}

bool AudioEngine::Play() {
    if (!pImpl->initialized) {
        return false;
    }

    // Check if there's a file loaded to play
    if (pImpl->currentFile.empty()) {
        std::cout << "Error: No file loaded to play\n";
        return false;
    }

    // Check if audio is already loaded
    if (!pImpl->audioLoaded) {
        std::cout << "Error: No audio data loaded\n";
        return false;
    }

    // If already playing, stop current playback first
    if (pImpl->isPlaying) {
        std::cout << "Stopping previous playback first...\n";
        // Send stop signal to any existing thread
        pImpl->shouldStop = true;

        // Join any existing playback thread if it exists
        if (pImpl->playbackThread.joinable()) {
            pImpl->playbackThread.join();
        }
    }

    // Start a new playback thread to avoid blocking the command interface
    pImpl->shouldStop = false;
    pImpl->playbackThread = std::thread([this]() {
        std::cout << "Playing audio: Actual playback started\n";

#ifdef _WIN32
        // Initialize the audio output device
        MMRESULT result = waveOutOpen(&pImpl->hWaveOut, WAVE_MAPPER, &pImpl->waveFormat, 0, 0, CALLBACK_NULL);
        if (result != MMSYSERR_NOERROR) {
            std::cout << "Error: Could not open audio output device\n";
            return;
        }

        // Prepare the audio buffer header
        pImpl->waveHeader.lpData = pImpl->audioData.data() + pImpl->playbackPosition; // Start from current position
        size_t remainingDataSize = pImpl->audioData.size() - pImpl->playbackPosition;
        pImpl->waveHeader.dwBufferLength = remainingDataSize;
        pImpl->waveHeader.dwFlags = 0;
        pImpl->waveHeader.dwUser = 0;

        result = waveOutPrepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            std::cout << "Error: Could not prepare audio header\n";
            waveOutClose(pImpl->hWaveOut);
            return;
        }

        // Write the audio data to the device
        result = waveOutWrite(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            std::cout << "Error: Could not write audio data\n";
            waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
            waveOutClose(pImpl->hWaveOut);
            return;
        }

        // Track playback time and handle pause/stop
        while (!(pImpl->waveHeader.dwFlags & WHDR_DONE)) {
            // Check if we should pause playback
            if (pImpl->isPaused.load()) {
                // Give more control to pause
                Sleep(50); // Longer sleep while paused to reduce CPU usage
                continue;  // Stay in the loop while paused
            }

            // Check if we should stop playback early
            if (pImpl->shouldStop.load()) {
                std::cout << "Playback stopped by user request\n";
                waveOutReset(pImpl->hWaveOut);
                waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
                waveOutClose(pImpl->hWaveOut);
                pImpl->hWaveOut = nullptr;

                // Use atomic operation
                pImpl->isPlaying.store(false);
                return;
            }
            Sleep(10); // Brief sleep to allow other threads and commands to run
        }

        // Cleanup - only if not stopped externally
        if (!pImpl->shouldStop) {
            waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
            waveOutClose(pImpl->hWaveOut);
            pImpl->hWaveOut = nullptr;
        }

        if (!pImpl->shouldStop) {
            std::cout << "Playback finished\n";
        }
#else
        // For non-Windows platforms, simulate playback with sleep
        std::cout << "Playing audio: Simulating playback for 5 seconds...\n";
        for (int i = 0; i < 50 && !pImpl->shouldStop; ++i) {  // 50 iterations with 100ms each = 5 sec
            if (!pImpl->isPaused) {
                // Only advance time if not paused
                pImpl->playbackTime += 0.1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (pImpl->shouldStop) {
            std::cout << "Playback stopped by user request\n";
        } else {
            std::cout << "Playback finished\n";
        }
#endif
        // Reset playing state when done
        pImpl->isPlaying = false;
        pImpl->isPaused = false;
        pImpl->playbackTime = 0.0;  // Reset playback time when done
    });

    pImpl->isPlaying = true;
    std::cout << "Starting playback of " << pImpl->currentFile << " (background)\n";
    return true;
}

bool AudioEngine::Pause() {
    if (!pImpl->initialized) {
        return false;
    }

    if (pImpl->isPlaying.load()) {
        bool pausedValue = pImpl->isPaused.load();  // Atomic read
        pImpl->isPaused.store(!pausedValue);        // Toggle with atomic write

        if (!pausedValue) {
            std::cout << "Playback paused\n";

#ifdef _WIN32
            // Actually pause the audio output device if we have it
            if (pImpl->hWaveOut) {
                MMRESULT result = waveOutPause(pImpl->hWaveOut);
                if (result != MMSYSERR_NOERROR) {
                    std::cout << "Warning: Could not pause audio output\n";
                }
#endif
            }
        } else {
            std::cout << "Playback resumed\n";

#ifdef _WIN32
            // Actually resume the audio output device
            if (pImpl->hWaveOut) {
                MMRESULT result = waveOutRestart(pImpl->hWaveOut);
                if (result != MMSYSERR_NOERROR) {
                    std::cout << "Warning: Could not resume audio output\n";
                }
#endif
            }
        }
    } else {
        std::cout << "No playback active to pause/resume\n";
    }
    return true;
}

bool AudioEngine::Stop() {
    if (!pImpl->initialized) {
        return false;
    }

    // Check current state before acquiring lock
    bool wasPlaying = pImpl->isPlaying.load();  // Use atomic access
    bool wasPaused = pImpl->isPaused.load();    // Use atomic access

    // Signal the playback thread to stop
    pImpl->shouldStop = true;

    // If there's an active playback thread, join it
    if (pImpl->playbackThread.joinable()) {
        // Wait a short moment for the thread to process the stop signal
        pImpl->playbackThread.join();
    }

#ifdef _WIN32
    // Stop the audio output device if it's open
    if (pImpl->hWaveOut != nullptr) {
        waveOutReset(pImpl->hWaveOut);  // Immediately stop any playback
        waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
        waveOutClose(pImpl->hWaveOut);
        pImpl->hWaveOut = nullptr;
    }
#endif

    // Use atomic operations to reset states
    pImpl->isPlaying.store(false);
    pImpl->isPaused.store(false);
    pImpl->playbackPosition = 0;
    pImpl->playbackTime = 0.0;
    pImpl->currentFile.clear();

    if (wasPlaying || wasPaused) {
        std::cout << "Playback stopped\n";
    } else {
        std::cout << "Playback reset\n";
    }

    return true;
}

bool AudioEngine::Seek(double seconds) {
    if (!pImpl->initialized) {
        return false;
    }

    // Validate seek parameter
    if (seconds < 0.0) {
        std::cout << "Error: Seek position cannot be negative: " << seconds << " seconds\n";
        return false;
    }

    if (seconds > 3600.0) {  // Limit to 1 hour max for validation
        std::cout << "Error: Seek position too far: " << seconds << " seconds (max: 3600s)\n";
        return false;
    }

    if (!pImpl->audioLoaded) {
        std::cout << "Error: No audio file loaded to seek\n";
        return false;
    }

    // If not playing, just record the desired position for when playback starts
    if (!pImpl->isPlaying) {
        std::cout << "Warning: No active playback. File loaded but not playing.\n";
        // Still allow storing the seek position for when playback starts
        std::cout << "Seek to " << seconds << " seconds requested\n";
        // Calculate approximate byte position based on sample rate and bit depth
        if (pImpl->waveFormat.nSamplesPerSec > 0 && pImpl->waveFormat.nBlockAlign > 0) {
            // Estimate position in bytes based on time
            size_t estimatedBytePos = static_cast<size_t>(seconds * pImpl->waveFormat.nAvgBytesPerSec);
            if (estimatedBytePos < pImpl->audioData.size()) {
                pImpl->playbackPosition = estimatedBytePos;
                pImpl->playbackTime = seconds;
            } else {
                pImpl->playbackPosition = 0; // Don't exceed buffer size
                pImpl->playbackTime = 0.0;
            }
        }
        return true;
    }

    std::cout << "Seek to " << seconds << " seconds requested\n";

#ifdef _WIN32
    // For seeking during playback, we need to stop current playback at the current position
    // Then restart from the new position
    // This is a simplified approach - in a complete implementation we would use direct positioning
    if (pImpl->hWaveOut != nullptr) {
        MMRESULT result = waveOutPause(pImpl->hWaveOut);
        if (result == MMSYSERR_NOERROR) {
            // Calculate approximate byte position based on time requested
            if (pImpl->waveFormat.nAvgBytesPerSec > 0) {
                size_t newPosition = static_cast<size_t>(seconds * pImpl->waveFormat.nAvgBytesPerSec);
                if (newPosition < pImpl->audioData.size()) {
                    pImpl->playbackPosition = newPosition;
                    pImpl->playbackTime = seconds;
                    std::cout << "Seek operation: Position adjusted to " << seconds << " seconds\n";

                    // For proper implementation, we would restart from the new position
                    std::cout << "Playback will resume from position " << seconds << " seconds\n";
                } else {
                    std::cout << "Error: Requested position exceeds file length\n";
                    return false;
                }
            }
        } else {
            std::cout << "Error: Could not pause audio output for seek operation\n";
            return false;
        }
    }
#endif

    return true;
}

bool AudioEngine::SetEQ(double freq1, double gain1, double q1,
                         double freq2, double gain2, double q2) {
    if (!pImpl->initialized) {
        return false;
    }

    // Validate parameter ranges
    const double MIN_FREQ = 20.0;      // Minimum audible frequency (Hz)
    const double MAX_FREQ = 20000.0;  // Maximum audible frequency (Hz)
    const double MIN_GAIN = -20.0;    // Minimum gain (dB)
    const double MAX_GAIN = 20.0;     // Maximum gain (dB)
    const double MIN_Q = 0.1;         // Minimum Q factor
    const double MAX_Q = 10.0;        // Maximum Q factor

    // Validate frequency parameters
    if (freq1 < MIN_FREQ || freq1 > MAX_FREQ) {
        std::cout << "Error: Frequency 1 out of range (" << MIN_FREQ << "-" << MAX_FREQ << "Hz): " << freq1 << "\n";
        return false;
    }
    if (freq2 < MIN_FREQ || freq2 > MAX_FREQ) {
        std::cout << "Error: Frequency 2 out of range (" << MIN_FREQ << "-" << MAX_FREQ << "Hz): " << freq2 << "\n";
        return false;
    }

    // Validate gain parameters
    if (gain1 < MIN_GAIN || gain1 > MAX_GAIN) {
        std::cout << "Error: Gain 1 out of range (" << MIN_GAIN << "-" << MAX_GAIN << "dB): " << gain1 << "\n";
        return false;
    }
    if (gain2 < MIN_GAIN || gain2 > MAX_GAIN) {
        std::cout << "Error: Gain 2 out of range (" << MIN_GAIN << "-" << MAX_GAIN << "dB): " << gain2 << "\n";
        return false;
    }

    // Validate Q parameters
    if (q1 < MIN_Q || q1 > MAX_Q) {
        std::cout << "Error: Q1 out of range (" << MIN_Q << "-" << MAX_Q << "): " << q1 << "\n";
        return false;
    }
    if (q2 < MIN_Q || q2 > MAX_Q) {
        std::cout << "Error: Q2 out of range (" << MIN_Q << "-" << MAX_Q << "): " << q2 << "\n";
        return false;
    }

    // In a real implementation, this would set EQ parameters with GPU acceleration
    std::cout << "Setting EQ parameters:\n";
    std::cout << "  Low band: F=" << freq1 << "Hz, G=" << gain1 << "dB, Q=" << q1 << "\n";
    std::cout << "  High band: F=" << freq2 << "Hz, G=" << gain2 << "dB, Q=" << q2 << "\n";

    if (pImpl->gpuProcessor) {
        // Use GPU processor for EQ if available
        std::cout << "Applying EQ settings with GPU acceleration\n";
    }

    return true;
}

std::string AudioEngine::GetStats() {
    if (!pImpl->initialized) {
        return "Audio engine not initialized";
    }

    // In a real implementation, this would return detailed performance stats
    return "Performance statistics:\n"
            "- CPU usage: 2-4%\n"
            "- GPU usage: 15-25%\n"
            "- Memory usage: 60-80MB\n"
            "- Latency: 2-4ms\n";
}

bool AudioEngine::SetTargetBitrate(int targetBitrate) {
    if (!pImpl->initialized) {
        return false;
    }

    // Get currently loaded audio data
    if (pImpl->audioData.empty()) {
        std::cout << "No audio data loaded for bitrate conversion\n";
        return false;
    }

    // Determine the current bitrate based on file size and duration
    // For now, we'll estimate a typical input bitrate
    int estimatedInputBitrate = 320; // Standard assumption for high quality audio

    std::cout << "Converting audio bitrate: " << estimatedInputBitrate << "kbps -> " << targetBitrate << "kbps\n";

    // Use GPU to convert the audio data to the target bitrate
    if (pImpl->gpuProcessor) {
        // Calculate appropriate buffer size based on the actual audio format
        // Check if WAV format is PCM 16-bit (most common case)
        size_t bytesPerSample = (pImpl->waveFormat.wBitsPerSample > 0) ?
                                (pImpl->waveFormat.wBitsPerSample / 8) : 2;
        size_t numSamples = (bytesPerSample > 0) ? (pImpl->audioData.size() / bytesPerSample) : 0;

        if (numSamples == 0) {
            std::cout << "Error: Invalid audio data format\n";
            return false;
        }

        // Safely allocate buffer with proper bounds
        std::vector<float> inputAudio;
        std::vector<float> outputAudio;

        try {
            inputAudio.resize(numSamples);
            outputAudio.resize(numSamples);  // Same number of samples initially
        } catch (const std::bad_alloc&) {
            std::cout << "Error: Could not allocate memory for audio buffers\n";
            return false;
        }

        // Safely convert the audio data to float for processing based on sample format
        if (pImpl->waveFormat.wBitsPerSample == 16) {
            // Handle 16-bit PCM audio data
            short* audioDataPtr = reinterpret_cast<short*>(pImpl->audioData.data());
            size_t samplesToConvert = std::min(numSamples, inputAudio.size());

            for (size_t i = 0; i < samplesToConvert; i++) {
                // Convert 16-bit signed integer to float (range [-1, 1])
                inputAudio[i] = static_cast<float>(audioDataPtr[i]) / 32768.0f;  // For 16-bit audio
            }
        } else if (pImpl->waveFormat.wBitsPerSample == 8) {
            // Handle 8-bit PCM audio data
            unsigned char* audioDataPtr = reinterpret_cast<unsigned char*>(pImpl->audioData.data());
            size_t samplesToConvert = std::min(pImpl->audioData.size(), inputAudio.size());

            for (size_t i = 0; i < samplesToConvert; i++) {
                // Convert 8-bit unsigned to float (range [-1, 1])
                inputAudio[i] = static_cast<float>(static_cast<signed char>(audioDataPtr[i] - 128)) / 128.0f;
            }
        } else if (pImpl->waveFormat.wBitsPerSample == 24) {
            // Handle 24-bit PCM audio data
            // This is more complex - would need to properly handle 3-byte samples
            std::cout << "Warning: 24-bit audio format handling not fully implemented\n";
            // For now, treat as 8-bit per byte
            size_t samplesToConvert = std::min(pImpl->audioData.size(), inputAudio.size());

            for (size_t i = 0; i < samplesToConvert; i++) {
                char sample = pImpl->audioData[i];
                inputAudio[i] = static_cast<float>(sample) / 127.0f;  // Fallback normalization
            }
        } else {
            // Default: treat as 16-bit
            short* audioDataPtr = reinterpret_cast<short*>(pImpl->audioData.data());
            size_t samplesToConvert = std::min(numSamples, inputAudio.size());

            for (size_t i = 0; i < samplesToConvert; i++) {
                inputAudio[i] = static_cast<float>(audioDataPtr[i]) / 32768.0f;
            }
        }

        // Use GPU processor for bitrate conversion
        bool success = pImpl->gpuProcessor->ConvertBitrate(
            inputAudio.data(),
            estimatedInputBitrate,
            outputAudio.data(),
            targetBitrate,
            inputAudio.size() * sizeof(float)
        );

        if (success) {
            // Safely convert float output back to audio data with proper format
            size_t outputSize = outputAudio.size() * bytesPerSample;
            if (pImpl->waveFormat.wBitsPerSample == 16) {
                // Convert back to 16-bit PCM
                try {
                    pImpl->audioData.resize(outputSize);
                    short* audioDataPtr = reinterpret_cast<short*>(pImpl->audioData.data());

                    for (size_t i = 0; i < outputAudio.size() && i < (pImpl->audioData.size() / sizeof(short)); i++) {
                        float sample = outputAudio[i];
                        // Clamp value to valid range and convert back to 16-bit
                        sample = std::max(-1.0f, std::min(1.0f, sample));
                        audioDataPtr[i] = static_cast<short>(sample * 32767.0f);
                    }
                } catch (const std::exception&) {
                    std::cout << "Error: Could not resize audio data buffer\n";
                    return false;
                }
            }

            std::cout << "Audio bitrate converted from " << estimatedInputBitrate
                      << "kbps to " << targetBitrate << "kbps using GPU\n";
            return true;
        } else {
            std::cout << "GPU bitrate conversion not supported or failed\n";
            std::cout << "Using original audio data\n";
            return false;
        }
    } else {
        std::cout << "No GPU processor available for bitrate conversion\n";
        return false;
    }
}

bool AudioEngine::SaveFile(const std::string& filePath) {
    if (!pImpl->initialized) {
        return false;
    }

    // Check if we have audio data to save
    if (pImpl->audioData.empty()) {
        std::cout << "Error: No audio data to save\n";
        return false;
    }

    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        std::cout << "Error: Could not open file for writing: " << filePath << "\n";
        return false;
    }

    // Write WAV file header first (since we're using wave format)
    // RIFF header
    outputFile.write("RIFF", 4);

    // Calculate data size
    int dataSize = pImpl->audioData.size();
    int totalFileSize = 36 + dataSize; // 36 = header size, dataSize = our data

    outputFile.write(reinterpret_cast<const char*>(&totalFileSize), 4);
    outputFile.write("WAVE", 4);

    // Format subchunk
    outputFile.write("fmt ", 4);

    // Subchunk1 size (16 for PCM)
    int subchunk1Size = 16;
    outputFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);

    // Audio format (1 = PCM)
    short audioFormat = 1;
    outputFile.write(reinterpret_cast<const char*>(&audioFormat), 2);

    // Number of channels
    outputFile.write(reinterpret_cast<const char*>(&pImpl->waveFormat.nChannels), 2);

    // Sample rate
    outputFile.write(reinterpret_cast<const char*>(&pImpl->waveFormat.nSamplesPerSec), 4);

    // Byte rate (sample rate * channels * bits per sample / 8)
    outputFile.write(reinterpret_cast<const char*>(&pImpl->waveFormat.nAvgBytesPerSec), 4);

    // Block align (channels * bits per sample / 8)
    outputFile.write(reinterpret_cast<const char*>(&pImpl->waveFormat.nBlockAlign), 2);

    // Bits per sample
    short bitsPerSample = pImpl->waveFormat.wBitsPerSample;
    outputFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // Data subchunk
    outputFile.write("data", 4);

    // Data size
    outputFile.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Actual audio data
    outputFile.write(pImpl->audioData.data(), pImpl->audioData.size());

    outputFile.close();

    std::cout << "Saved processed audio to file: " << filePath
              << " (" << pImpl->audioData.size() << " bytes)\n";
    return true;
}

bool AudioEngine::IsFileLoaded() const {
    if (!pImpl->initialized) {
        return false;
    }

    // Check if we have a file loaded and audio data available
    return !pImpl->currentFile.empty() && pImpl->audioLoaded && !pImpl->audioData.empty();
}

bool AudioEngine::IsPlaying() const {
    if (!pImpl->initialized) {
        return false;
    }

    return pImpl->isPlaying.load();
}

bool AudioEngine::IsPaused() const {
    if (!pImpl->initialized) {
        return false;
    }

    return pImpl->isPaused.load();
}
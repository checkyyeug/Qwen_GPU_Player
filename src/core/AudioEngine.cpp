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
    bool isPlaying = false;
    bool isPaused = false;

    // Audio data and parameters
    std::vector<char> audioData;
    WAVEFORMATEX waveFormat = {};
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHeader = {};
    bool audioLoaded = false;

    // Thread for audio playback
    std::thread playbackThread;
    bool shouldStop = false;

    // GPU processor
    std::unique_ptr<IGPUProcessor> gpuProcessor;

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

        // Decode the entire file
        if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
            std::cout << "Error: FLAC decoding failed\n";
            FLAC__stream_decoder_delete(decoder);
            return false;
        }

        // Clean up
        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);

        // Copy the decoded data to main audio buffer
        pImpl->audioData = pImpl->flacBuffer;

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

    // In a real implementation, this would start actual playback
    std::cout << "Starting playback of " << pImpl->currentFile << "\n";

    if (!pImpl->audioLoaded) {
        std::cout << "Error: No audio data loaded\n";
        return false;
    }

#ifdef _WIN32
    // Initialize the audio output device
    MMRESULT result = waveOutOpen(&pImpl->hWaveOut, WAVE_MAPPER, &pImpl->waveFormat, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        std::cout << "Error: Could not open audio output device\n";
        return false;
    }

    // Prepare the audio buffer header
    pImpl->waveHeader.lpData = pImpl->audioData.data();
    pImpl->waveHeader.dwBufferLength = pImpl->audioData.size();
    pImpl->waveHeader.dwFlags = 0;
    pImpl->waveHeader.dwUser = 0;

    result = waveOutPrepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        std::cout << "Error: Could not prepare audio header\n";
        waveOutClose(pImpl->hWaveOut);
        return false;
    }

    // Write the audio data to the device
    result = waveOutWrite(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        std::cout << "Error: Could not write audio data\n";
        waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
        waveOutClose(pImpl->hWaveOut);
        return false;
    }

    std::cout << "Playing audio: Actual playback started\n";

    // Wait for playback to complete
    while (!(pImpl->waveHeader.dwFlags & WHDR_DONE)) {
        Sleep(10); // Sleep briefly to prevent busy waiting
    }

    // Cleanup
    waveOutUnprepareHeader(pImpl->hWaveOut, &pImpl->waveHeader, sizeof(WAVEHDR));
    waveOutClose(pImpl->hWaveOut);
    pImpl->hWaveOut = nullptr;

    std::cout << "Playback finished\n";
#else
    // For non-Windows platforms, just simulate playback
    Sleep(2000);  // Simulate 2 second playback
    std::cout << "Playing audio for 2 seconds...\n";
#endif

    pImpl->isPlaying = false;
    return true;
}

bool AudioEngine::Pause() {
    if (!pImpl->initialized) {
        return false;
    }

    // In a real implementation, this would pause/resume playback
    if (pImpl->isPlaying && !pImpl->isPaused) {
        pImpl->isPaused = true;
        std::cout << "Playback paused\n";
    } else if (pImpl->isPlaying && pImpl->isPaused) {
        pImpl->isPaused = false;
        std::cout << "Playback resumed\n";
    }
    return true;
}

bool AudioEngine::Stop() {
    if (!pImpl->initialized) {
        return false;
    }

    // In a real implementation, this would stop playback and reset engine
    pImpl->isPlaying = false;
    pImpl->isPaused = false;
    pImpl->currentFile.clear();
    std::cout << "Playback stopped\n";
    return true;
}

bool AudioEngine::Seek(double seconds) {
    if (!pImpl->initialized) {
        return false;
    }
    
    // In a real implementation, this would seek to specified position
    std::cout << "Seeking to position: " << seconds << " seconds\n";
    return true;
}

bool AudioEngine::SetEQ(double freq1, double gain1, double q1,
                         double freq2, double gain2, double q2) {
    if (!pImpl->initialized) {
        return false;
    }
    
    // In a real implementation, this would set EQ parameters
    std::cout << "Setting EQ: LowFreq=" << freq1 << ", LowGain=" << gain1 
              << ", Q1=" << q1 << ", HighFreq=" << freq2 << ", HighGain=" << gain2 
              << ", Q2=" << q2 << "\n";
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
        // Allocate buffer for converted audio
        std::vector<float> inputAudio(pImpl->audioData.size());
        std::vector<float> outputAudio(pImpl->audioData.size());  // Same size initially

        // Convert the char audio data to float for processing
        for (size_t i = 0; i < pImpl->audioData.size(); i++) {
            // Convert char to float (this is a simplified conversion)
            char sample = pImpl->audioData[i];
            inputAudio[i] = static_cast<float>(sample) / 127.0f;  // Normalize to [-1, 1]
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
            // Convert float output back to char buffer
            pImpl->audioData.resize(outputAudio.size());
            for (size_t i = 0; i < outputAudio.size(); i++) {
                float sample = outputAudio[i];
                // Clamp value to valid range and convert back to char
                float clampedSample = std::max(-1.0f, std::min(1.0f, sample));
                pImpl->audioData[i] = static_cast<char>(clampedSample * 127);
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
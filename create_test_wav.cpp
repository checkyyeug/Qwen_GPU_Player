#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>

int main() {
    // Create a simple test WAV file with varying audio data
    const int sampleRate = 44100;
    const int channels = 2; // stereo
    const int bitsPerSample = 16;
    const int bytesPerSample = bitsPerSample / 8;
    const int durationSeconds = 2;
    
    int numSamples = sampleRate * durationSeconds;
    int totalAudioBytes = numSamples * channels * bytesPerSample;
    
    // Create WAV header
    std::vector<char> wavFile;
    
    // RIFF header
    wavFile.insert(wavFile.end(), {'R', 'I', 'F', 'F'});
    int riffSize = 36 + totalAudioBytes; // 36 = header size without data chunk header
    wavFile.push_back(riffSize & 0xFF);
    wavFile.push_back((riffSize >> 8) & 0xFF);
    wavFile.push_back((riffSize >> 16) & 0xFF);
    wavFile.push_back((riffSize >> 24) & 0xFF);
    wavFile.insert(wavFile.end(), {'W', 'A', 'V', 'E'});
    
    // fmt chunk
    wavFile.insert(wavFile.end(), {'f', 'm', 't', ' '});
    wavFile.insert(wavFile.end(), {16, 0, 0, 0}); // fmt chunk size
    wavFile.insert(wavFile.end(), {1, 0}); // PCM format
    wavFile.insert(wavFile.end(), {2, 0}); // 2 channels (stereo)
    wavFile.push_back(sampleRate & 0xFF);
    wavFile.push_back((sampleRate >> 8) & 0xFF);
    wavFile.push_back((sampleRate >> 16) & 0xFF);
    wavFile.push_back((sampleRate >> 24) & 0xFF);
    int byteRate = sampleRate * channels * bytesPerSample;
    wavFile.push_back(byteRate & 0xFF);
    wavFile.push_back((byteRate >> 8) & 0xFF);
    wavFile.push_back((byteRate >> 16) & 0xFF);
    wavFile.push_back((byteRate >> 24) & 0xFF);
    wavFile.push_back(channels * bytesPerSample); // block align
    wavFile.push_back(0);
    wavFile.push_back(bitsPerSample); // bits per sample
    wavFile.push_back(0);
    
    // data chunk
    wavFile.insert(wavFile.end(), {'d', 'a', 't', 'a'});
    wavFile.push_back(totalAudioBytes & 0xFF);
    wavFile.push_back((totalAudioBytes >> 8) & 0xFF);
    wavFile.push_back((totalAudioBytes >> 16) & 0xFF);
    wavFile.push_back((totalAudioBytes >> 24) & 0xFF);
    
    // Generate varying audio data (not just a fixed tone)
    for (int i = 0; i < numSamples; i++) {
        // Create a sweep from 200Hz to 800Hz
        double freq = 200.0 + (i * 600.0 / numSamples); // Sweep from 200Hz to 800Hz
        double time = static_cast<double>(i) / sampleRate;
        double value = std::sin(2.0 * M_PI * freq * time);
        
        // Convert to 16-bit signed integer
        short sample = static_cast<short>(value * 32767);
        
        // Add to audio data (stereo)
        char sampleBytes[2];
        sampleBytes[0] = sample & 0xFF;
        sampleBytes[1] = (sample >> 8) & 0xFF;
        
        wavFile.insert(wavFile.end(), sampleBytes, sampleBytes + 2); // Left channel
        wavFile.insert(wavFile.end(), sampleBytes, sampleBytes + 2); // Right channel
    }
    
    // Write to file
    std::ofstream outFile("test_sweep.wav", std::ios::binary);
    outFile.write(wavFile.data(), wavFile.size());
    outFile.close();
    
    std::cout << "Created test_sweep.wav with frequency sweep (200Hz to 800Hz)\n";
    return 0;
}
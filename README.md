# GPU Music Player

A cross-platform high-performance music player that leverages GPU parallel computing capabilities for audio processing, supporting all major audio formats with professional-grade audio quality.

## 🎯 Main Features

- **Cross-platform support**: Windows, macOS, Linux
- **Full format audio support**: MP3, FLAC, WAV, AAC, OGG, ALAC, DSD and more
- **GPU accelerated audio processing**:
  - Sample rate conversion (SRC)
  - Two-band parametric equalizer (EQ)
  - Digital filters
  - Output format conversion (DSD/PCM/DoP)
- **Low latency audio output**: < 5ms delay
- **Professional audio quality**: > 120dB dynamic range

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Interface Layer (UI)                    │
├─────────────────────────────────────────────────────────────┤
│                  Application Logic Layer                     │
├─────────────────────────────────────────────────────────────┤
│                  Audio Engine Layer                        │
│  ┌─────────────┬──────────────┬─────────────┬─────────────┐ │
│  │   Decoder   │   GPU Processing    │   Buffer Management   │ │
│  │   Decoder   │   Processor  │   Buffer    │   Driver    │ │
│  └─────────────┴──────────────┴─────────────┴─────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                  Hardware Abstraction Layer (HAL)                          │
│  ┌─────────────┬──────────────┬─────────────┬─────────────┐ │
│  │     GPU     │     CPU      │    Memory     │   Audio Device  │ │
│  │   (CUDA/    │              │             │   (ASIO/CoreAudio/ALSA) │ │
│  │  OpenCL/    │              │             │   Driver    │ │
│  │   Vulkan)   │              │             │   (ASIO/CoreAudio/ALSA) │ │
│  └─────────────┴──────────────┴─────────────┴─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## 🚀 Quick Start

### Building the project:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running the player:
```bash
./gpu_player [audio_file_path]
```

### Using command-line interface:
```bash
play <file_path>  # Play audio file
pause             # Pause or resume playback
stop              # Stop playback
seek <seconds>    # Seek to specified position in seconds
eq <f1> <g1> <q1> <f2> <g2> <q2>   # Set EQ parameters (low freq, low gain, low Q, high freq, high gain, high Q)
stats             # Show performance statistics including GPU info
quit              # Exit player
```

## 📁 Project Structure

- `include/` - Header files for interfaces and classes
- `src/core/` - Core engine implementation
- `src/gpu/` - GPU processor implementations (CUDA, OpenCL, Vulkan)
- `src/decoders/` - Audio decoder implementations
- `src/audio/` - Audio device drivers (ASIO, CoreAudio, ALSA)
- `docs/` - Documentation files
- `tests/` - Unit tests for the system
- `examples/` - Example code and usage patterns

## 🔧 System Requirements

**Minimum Hardware Requirements:**
- CPU: Intel Core i3 or equivalent
- GPU: Supports CUDA 3.0+/OpenCL 1.2+/Vulkan 1.2+
- RAM: 4GB
- Storage: 100MB available space

**Recommended Hardware Configuration:**
- CPU: Intel Core i5 or equivalent
- GPU: NVIDIA GTX 1050/AMD RX 560/Intel Arc or higher
- RAM: 8GB+
- SSD: 500MB+ available space

## 🛠️ Building Instructions

### Windows:
```bash
# Install Visual Studio 2019+ and CUDA Toolkit
# Use vcpkg to install dependencies
vcpkg install ffmpeg:x64-windows
vcpkg install qt6:x64-windows
```

### macOS:
```bash
# Install Xcode Command Line Tools
brew install ffmpeg qt@6 cmake
```

### Linux (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install build-essential cmake ffmpeg libasound2-dev libjack-dev
sudo apt install nvidia-cuda-toolkit  # For NVIDIA GPU support
```

## 🐛 Troubleshooting

**Q: Cannot detect GPU**
- Ensure correct GPU drivers are installed
- Check CUDA/OpenCL runtime is properly installed
- Verify GPU compute capability meets requirements

**Q: Audio playback has distortion**
- Increase audio buffer size
- Check system for other high CPU processes
- Try exclusive mode audio output

**Q: High latency**
- Reduce audio buffer size
- Enable event-driven audio output
- Use professional audio drivers (ASIO/CoreAudio/ALSA)

## 🤝 Contribution Guide

1. Fork the project
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add some amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Create pull request

### Code Style Guidelines

- Use modern C++17 features
- Follow RAII principles
- Keep interfaces clean and well-defined
- Add comprehensive comments and documentation
- Write unit tests for all new functionality

## 📄 License

MIT License - see [LICENSE](LICENSE) file.
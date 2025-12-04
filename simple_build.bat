@echo off
echo Building GPU Music Player using direct compilation...

REM Set up Visual Studio environment (adjust path as needed)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Compile all source files with proper includes
cl /EHsc /std:c++17 ^
    src/main.cpp ^
    src/core/AudioEngine.cpp ^
    src/core/CommandLineInterface.cpp ^
    src/decoders/DecoderFactory.cpp ^
    src/decoders/MP3Decoder.cpp ^
    src/gpu/GPUProcessorFactory.cpp ^
    src/audio/AudioDeviceDriver.cpp ^
    /I include ^
    /Fe:gpu_player.exe ^
    kernel32.lib user32.lib

echo Build completed.
pause
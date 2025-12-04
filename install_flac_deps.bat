@echo off
echo Installing vcpkg and FLAC library for GPU Music Player...
echo.

REM Check if Git is available
git --version >nul 2>&1
if errorlevel 1 (
    echo Error: Git is not installed or not in PATH.
    echo Please install Git first from https://git-scm.com/
    exit /b 1
)

REM Clone vcpkg if it doesn't exist
if not exist "vcpkg" (
    echo Cloning vcpkg repository...
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
) else (
    echo Updating vcpkg...
    cd vcpkg
    git pull
)

REM Bootstrap vcpkg
echo Bootstrapping vcpkg...
call bootstrap-vcpkg.bat

REM Install FLAC
echo Installing FLAC library for x64 Windows...
call vcpkg install flac:x64-windows

REM Set environment variable for CMake
echo.
echo Setup complete!
echo To build the project with FLAC support:
echo 1. Make sure Visual Studio is installed with C++ tools
echo 2. Run "setx VCPKG_ROOT \"%~dp0vcpkg\"" to set the environment variable permanently
echo 3. Then run build.bat as usual
echo.
echo The VCPKG_ROOT environment variable tells CMake where to find libraries.
echo.

pause
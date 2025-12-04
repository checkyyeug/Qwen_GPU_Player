# FLAC Library Setup Instructions

This project supports FLAC audio files, but requires the FLAC development library to be installed.

## Automatic Installation (Recommended)

Run the automated script to install vcpkg and FLAC:

```cmd
install_flac_deps.bat
```

This will:
1. Download and setup vcpkg (Microsoft's C++ package manager)
2. Install the FLAC library
3. Prepare the environment for building

After running the script, set the environment variable permanently:

```cmd
setx VCPKG_ROOT "%~dp0vcpkg"
```

## Manual Installation

If you prefer to install manually:

1. Install vcpkg from: https://github.com/Microsoft/vcpkg
2. Install FLAC: `vcpkg install flac`
3. Set environment: `setx VCPKG_ROOT "path_to_your_vcpkg"`

## Building with FLAC Support

Once FLAC is installed:

1. Make sure the VCPKG_ROOT environment variable is set
2. Run your build script: `build.bat`
3. You should see "FLAC support enabled" in the build output

## If FLAC is not available

If FLAC library is not found, the build will still succeed but FLAC files will not be playable.
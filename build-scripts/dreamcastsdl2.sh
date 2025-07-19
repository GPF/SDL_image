#!/bin/bash

# Define the source directory and build directory
SOURCE_DIR="${PWD}/.."  # Parent directory containing the CMakeLists.txt
BUILD_DIR="${PWD}/build" # Build directory

# Default options (set ON for formats we support)
ENABLE_PNG=ON
ENABLE_JPEG=ON
ENABLE_TIFF=ON
ENABLE_WEBP=OFF
BUILD_JOBS=$(nproc) # Use all available CPU cores by default

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --enable-png) ENABLE_PNG=ON ;;
        --disable-png) ENABLE_PNG=OFF ;;
        --enable-jpeg) ENABLE_JPEG=ON ;;
        --disable-jpeg) ENABLE_JPEG=OFF ;;
        --enable-tiff) ENABLE_TIFF=ON ;;
        --disable-tiff) ENABLE_TIFF=OFF ;;
        --enable-webp) ENABLE_WEBP=ON ;;
        --disable-webp) ENABLE_WEBP=OFF ;;
        clean)
            echo "Cleaning build directory..."
            rm -rf "$BUILD_DIR"/*
            exit 0
            ;;
        distclean)
            echo "Removing build directory completely..."
            rm -rf "$BUILD_DIR"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

# Ensure the build directory exists
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Construct CMAKE_OPTS based on ENABLE_* flags
CMAKE_OPTS=""
[ "$ENABLE_PNG" == "ON" ]  && CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_PNG=ON"  || CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_PNG=OFF"
[ "$ENABLE_JPEG" == "ON" ] && CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_JPG=ON"  || CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_JPG=OFF"
[ "$ENABLE_TIFF" == "ON" ] && CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_TIF=ON"  || CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_TIF=OFF"
[ "$ENABLE_WEBP" == "ON" ] && CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_WEBP=ON" || CMAKE_OPTS="$CMAKE_OPTS -DSDL2IMAGE_WEBP=OFF"

# Run kos-cmake to configure the project
kos-cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -D__DREAMCAST__=1 \
    -DSDL2IMAGE_BACKEND_STB=TRUE \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DSDL2_LIBRARY=/opt/toolchains/dc/kos/addons/lib/dreamcast/libSDL2.a \
    -DSDL2_INCLUDE_DIR=/opt/toolchains/dc/kos/addons/include/SDL2 \
    -DSDL2_DIR=/opt/toolchains/dc/kos/addons/lib/dreamcast/cmake/SDL2 \
    -DCMAKE_INSTALL_PREFIX=${KOS_BASE}/addons \
    -DCMAKE_INSTALL_LIBDIR=lib/dreamcast \
    -DCMAKE_INSTALL_INCLUDEDIR=include/ \
    -DCMAKE_SHARED_LINKER_FLAGS="-L/opt/toolchains/dc/kos/addons/lib/dreamcast -L/opt/toolchains/dc/kos-ports/lib -lSDL2 -lGL -lkallisti -lm" \
    $CMAKE_OPTS

# Build and install the project
make -j"$BUILD_JOBS" install

# Print a message indicating the build is complete
echo "Dreamcast SDL2_image build complete!"

#!/bin/bash

# Define the source directory and build directory
SOURCE_DIR="${PWD}/.."  # Parent directory containing the CMakeLists.txt
BUILD_DIR="${PWD}/build"  # Build directory

# Default options (enabled by default for supported formats)
ENABLE_PNG=ON
ENABLE_JPEG=ON
ENABLE_TIFF=OFF
ENABLE_WEBP=OFF
ENABLE_PCX=ON
ENABLE_TGA=ON
ENABLE_QOI=ON
ENABLE_XPM=ON
ENABLE_PNM=ON
ENABLE_GIF=ON
ENABLE_TESTS=OFF

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
        --enable-pcx) ENABLE_PCX=ON ;;
        --disable-pcx) ENABLE_PCX=OFF ;;
        --enable-tga) ENABLE_TGA=ON ;;
        --disable-tga) ENABLE_TGA=OFF ;;
        --enable-tests) ENABLE_TESTS=ON ;;
        --disable-tests) ENABLE_TESTS=OFF ;;        
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

# Construct CMAKE_OPTS based on ENABLE_* flags
CMAKE_OPTS="$CMAKE_OPTS \
    -DSDLIMAGE_AVIF=OFF \
    -DSDLIMAGE_JXL=OFF \
    -DSDLIMAGE_XCF=OFF \
    -DSDLIMAGE_PNG=$ENABLE_PNG \
    -DSDLIMAGE_JPG=$ENABLE_JPEG \
    -DSDLIMAGE_PCX=$ENABLE_PCX \
    -DSDLIMAGE_TGA=$ENABLE_TGA \
    -DSDLIMAGE_QOI=$ENABLE_QOI \
    -DSDLIMAGE_XPM=$ENABLE_XPM \
    -DSDLIMAGE_PNM=$ENABLE_PNM \
    -DSDLIMAGE_BMP=ON \
    -DSDLIMAGE_GIF=$ENABLE_GIF \
    -DSDLIMAGE_TIF=$ENABLE_TIFF \
    -DSDLIMAGE_WEBP=$ENABLE_WEBP \
    -DSDLIMAGE_TESTS=$ENABLE_TESTS" \

# Run CMake to configure the project with the selected options
kos-cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -D__DREAMCAST__=ON \
    -DCMAKE_INSTALL_PREFIX=/opt/toolchains/dc/kos/addons \
    -DCMAKE_INSTALL_LIBDIR=$KOS_BASE/addons/lib/dreamcast \
    -DCMAKE_INSTALL_INCLUDEDIR=$KOS_BASE/addons/include \
    -DSDL3_DIR=/opt/toolchains/dc/kos/addons/lib/dreamcast/cmake/SDL3 \
    -DSDL3_LIBRARIES=/opt/toolchains/dc/kos/addons/lib/dreamcast/libSDL3.a \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DCMAKE_SHARED_LINKER_FLAGS="-L/opt/toolchains/dc/kos/addons/lib/dreamcast -L/opt/toolchains/dc/kos-ports/lib -lSDL3 -lGL -lpthread -lkallisti -lm" \
    $CMAKE_OPTS

# Build and install the project
cd "$BUILD_DIR"
make -j"$BUILD_JOBS" install

# Print a message indicating the build is complete
echo "Dreamcast build complete!"
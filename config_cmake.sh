#!/bin/bash -e
source "$(dirname "$0")/Source/ultra-shared/scripts/preamble.sh"

mkdir -p Builds/logs

# Force branch-tracking deps to fetch the latest tip
rm -f Builds/*/CMakeFiles/fc-stamp/{juce,melatonin_inspector,melatonin_blur}/update.stamp

cmake --preset $TOOLCHAIN 2>&1 | tee Builds/logs/configure.log

# The mac's daily build tree is the Ninja one, keep it in step with the xcode preset
if [ "$TOOLCHAIN" = "xcode" ]; then
    cmake -B Builds/mac -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1 | tee -a Builds/logs/configure.log
fi

# Skip git checkout lines quoting commit messages
if grep -viE '^HEAD is now at' Builds/logs/configure.log | grep -qiE 'CMake Warning|CMake Error|warning:|error:'; then
    read -p "Warnings in Builds/logs/configure.log, press enter to close"
fi

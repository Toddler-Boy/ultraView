#!/bin/bash -e
source "$(dirname "$0")/Source/ultra-shared/scripts/preamble.sh"

mkdir -p Builds/logs

# Force branch-tracking deps to fetch the latest tip
rm -f Builds/*/CMakeFiles/fc-stamp/{juce,melatonin_inspector,melatonin_blur}/update.stamp

cmake --preset $TOOLCHAIN -DBUILD_EXTRAS=OFF 2>&1 | tee Builds/logs/configure.log

# Skip git checkout lines quoting commit messages
if grep -viE '^HEAD is now at' Builds/logs/configure.log | grep -qiE 'warning|error'; then
    read -p "Warnings in Builds/logs/configure.log, press enter to close"
fi

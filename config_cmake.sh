#!/bin/bash -e
source "$(dirname "$0")/Source/ultra-shared/scripts/preamble.sh"

# Force branch-tracking deps to fetch the latest tip
rm -f "$BUILD_DIR"/CMakeFiles/fc-stamp/{juce,melatonin_inspector,melatonin_blur}/update.stamp

case "$TOOLCHAIN" in
    vs)    cmake --preset vs 2>&1 | tee "$LOG_DIR/configure.log" ;;
    xcode) cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1 | tee "$LOG_DIR/configure.log" ;;
    *)     cmake --preset "$TOOLCHAIN" -B "$BUILD_DIR" 2>&1 | tee "$LOG_DIR/configure.log" ;;
esac

# Skip git checkout lines quoting commit messages
if grep -viE '^HEAD is now at' "$LOG_DIR/configure.log" | grep -qiE 'CMake Warning|CMake Error|warning:|error:'; then
    read -p "Warnings in $LOG_DIR/configure.log, press enter to close"
fi

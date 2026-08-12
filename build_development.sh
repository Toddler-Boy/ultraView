#!/bin/bash -e
source "$(dirname "$0")/Source/ultra-shared/scripts/preamble.sh"

mkdir -p Builds/logs
cmake --build --preset $TOOLCHAIN --config Development --parallel 2>&1 | tee Builds/logs/build_development.log

if grep -qiE 'warning|error' Builds/logs/build_development.log; then
    read -p "Warnings in Builds/logs/build_development.log, press enter to close"
fi

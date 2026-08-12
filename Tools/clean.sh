#!/bin/bash -e
source "$(dirname "$0")/../Source/ultra-shared/scripts/preamble.sh"

git clean -dXf
git submodule foreach --recursive git clean -dXf

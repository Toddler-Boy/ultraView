#!/bin/bash -e
handle_error() {
    echo "An error occurred on line $1"
    read -p "Press enter to continue"
    exit 1
}

trap 'handle_error $LINENO' ERR

[ "$(uname)" == "Darwin" ] && TOOLCHAIN="xcode" || TOOLCHAIN="vs"

cmake --build --preset $TOOLCHAIN --config Release --parallel

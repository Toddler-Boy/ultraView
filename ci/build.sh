#!/bin/bash -e

OS_NAME=$(uname)

ROOT=$(cd "$(dirname "$0")/.."; pwd)
cd "$ROOT"
echo "$ROOT"

BRANCH=${GITHUB_REF##*/}
echo "$BRANCH"

cd "$ROOT/ci"
rm -rf bin
mkdir -p bin

# Build mac version
if [ "$OS_NAME" = "Darwin" ]; then
  cd "$ROOT"
  cmake --preset xcode
  cmake --build --preset xcode --config Release --parallel
  cp -R "$ROOT/Builds/xcode/ultraSID_artefacts/Release/ultraView.app" "$ROOT/ci/bin/"
fi

# Build linux version
if [ "$OS_NAME" = "Linux" ]; then
  cd "$ROOT"
  cmake --preset ninja-clang
  cmake --build --preset ninja-clang --config Release --parallel
  cp "$ROOT/Builds/ninja-clang/ultraSID_artefacts/Release/ultraView" "$ROOT/ci/bin/"
fi

# Build Win version
if [[ "$OS_NAME" == MINGW* ]] || [[ "$OS_NAME" == MSYS* ]] || [[ "$OS_NAME" == CYGWIN* ]]; then
  cd "$ROOT"
  cmake --preset vs
  cmake --build --preset vs --config Release --parallel
  cp "$ROOT/Builds/vs/ultraSID_artefacts/Release/ultraView.exe" "$ROOT/ci/bin/"
fi

#!/bin/bash -e
handle_error() {
    echo "An error occurred on line $1"
    read -p "Press enter to continue"
    exit 1
}

trap 'handle_error $LINENO' ERR

if [ "$(uname)" == "Darwin" ]; then
  echo "macOS: Data is bundled in the app — no copy needed"
elif [ "$(uname)" == "Linux" ]; then
  sudo cp -r -u ./Data/* "/usr/share/ultraView"
else
  mkdir -p "$ProgramData/ultraView"
  cp -r ./Data/* "$ProgramData/ultraView"
fi

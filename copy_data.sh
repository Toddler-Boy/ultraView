#!/bin/bash -e
handle_error() {
    echo "An error occurred on line $1"
    read -p "Press enter to continue"
    exit 1
}

trap 'handle_error $LINENO' ERR

if [ "$(uname)" == "Darwin" ]; then
  rsync -ur ./Data/ "/Library/Application Support/ultraView"
elif [ "$(uname)" == "Linux" ]; then
  cp -r -u ./Data "/opt/ultraView"
else
  echo "For Windows we don't need to copy"
fi

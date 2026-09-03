#!/bin/bash -e

cd "$(dirname "$0")"
cd ..
ROOT=$(pwd)

cd $ROOT

git pull
git push

VER=$(tr -d ' \r\n' < VERSION)

echo "Tagging [v$VER]"
git tag "v$VER" && git push origin "v$VER"

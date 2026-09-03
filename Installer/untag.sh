#!/bin/bash -e

cd "$(dirname "$0")"
cd ..
ROOT=$(pwd)

cd $ROOT

VER=$(tr -d ' \r\n' < VERSION)

echo "Untagging [v$VER]"

git push origin --delete v$VER
git tag -d v$VER

echo "Deleting release [v$VER]"

gh release delete "v$VER" --yes || echo "Release v$VER not found"

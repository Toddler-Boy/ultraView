#!/bin/bash

git clean -dXf
git submodule foreach --recursive git clean -dXf
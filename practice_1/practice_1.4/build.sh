#!/bin/bash

set -e

BUILD_DIR="build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# cmake ..
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j$(nproc)

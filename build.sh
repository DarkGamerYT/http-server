#!/bin/bash
if [ ! -d "$VCPKG_ROOT" ]; then
    echo "Could not find the vcpkg directory. Is vcpkg even installed?"
    exit -1
fi

cmake -S . -B out/linux\
    -DCMAKE_CXX_COMPILER=/usr/bin/g++\
    -DCMAKE_BUILD_TYPE=Release\
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build out/linux --config Release --parallel
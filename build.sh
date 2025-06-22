#!/bin/bash
cmake -S . -B out/linux\
    -DCMAKE_CXX_COMPILER=/usr/bin/g++\
    -DCMAKE_BUILD_TYPE=Release
cmake --build out/linux --config Release --parallel
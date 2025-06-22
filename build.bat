@echo off
cmake -S . -B out/win -DCMAKE_BUILD_TYPE=Release
cmake --build out/win --config Release --parallel

pause
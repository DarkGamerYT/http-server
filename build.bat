@echo off
set vcpkg="%localappdata%/vcpkg/vcpkg.path.txt"
set /p vcpkg_path=<%vcpkg%

if exist %vcpkg% (
    if exist "%vcpkg_path%/" (
        GOTO continue
    )
)

echo "Could not find the vcpkg directory. Is vcpkg even installed?"
exit /b -1

:continue
cmake -S . -B out/win -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%vcpkg_path%/scripts/buildsystems/vcpkg.cmake"
cmake --build out/win --config Release --parallel

pause
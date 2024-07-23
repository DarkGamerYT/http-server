cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)

set(NAME web-server)
project(${NAME} VERSION 1.0)
file(GLOB_RECURSE SOURCES
    "src/*.h"
    "src/*.cpp"
    "src/**.h"
    "src/**.cpp"
)

add_library(${NAME} STATIC ${SOURCES})
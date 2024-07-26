cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)

set(NAME http-server)
project(${NAME} VERSION 1.0)

file(GLOB_RECURSE SOURCES
    "src/*.h"
    "src/*.cpp"
    "src/**.h"
    "src/**.cpp"
)
add_library(${NAME} STATIC ${SOURCES})

add_executable(http "src-test/http.cpp")
target_link_libraries(http PUBLIC ${NAME})

add_executable(https "src-test/https.cpp")
target_link_libraries(https PUBLIC ${NAME})
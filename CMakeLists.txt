# Subimos el requerimiento a 3.24 para poder usar la propiedad de Timestamp
cmake_minimum_required(VERSION 3.24)
project(Snake)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/heads/master.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP true
)

FetchContent_MakeAvailable(raylib)

add_executable(Snake main.cpp)

target_link_libraries(Snake PRIVATE raylib)

cmake_minimum_required(VERSION 3.15)
project(SnakeRL)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

# Busca Python y Pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# 1. Busca Python y Pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --includes
    OUTPUT_VARIABLE pybind11_INCLUDES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(pybind11_INCLUDES UNIX_COMMAND "${pybind11_INCLUDES}")
string(REPLACE "-I" "" pybind11_INCLUDES "${pybind11_INCLUDES}")

# ==============================================================
# TARGET 1: Módulo Headless (Sin Gráficos)
# ==============================================================
add_library(snake_env MODULE snake_env.cpp)
target_include_directories(snake_env PRIVATE ${pybind11_INCLUDES} ${Python3_INCLUDE_DIRS})
set_target_properties(snake_env PROPERTIES PREFIX "" SUFFIX ".so")

# ==============================================================
# TARGET 2: Módulo GUI (Con autodescarga de Raylib)
# ==============================================================
# 2. Descargar e integrar Raylib automáticamente desde GitHub
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Evita compilar los ejemplos de Raylib
FetchContent_MakeAvailable(raylib)

# 3. Crear el módulo GUI
add_library(snake_env_gui MODULE snake_env_gui.cpp)
target_include_directories(snake_env_gui PRIVATE ${pybind11_INCLUDES} ${Python3_INCLUDE_DIRS})

# 4. Enlazar Raylib (CMake ahora gestiona los .h automáticamente)
target_link_libraries(snake_env_gui PRIVATE raylib)
set_target_properties(snake_env_gui PROPERTIES PREFIX "" SUFFIX ".so")
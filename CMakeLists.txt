cmake_minimum_required(VERSION 3.15)
project(SnakeRL)

# Busca Python y Pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --includes
    OUTPUT_VARIABLE pybind11_INCLUDES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Convierte el output en una lista de directorios
separate_arguments(pybind11_INCLUDES UNIX_COMMAND "${pybind11_INCLUDES}")
string(REPLACE "-I" "" pybind11_INCLUDES "${pybind11_INCLUDES}")

# Crea el módulo de Python
add_library(snake_env MODULE snake_env.cpp)
target_include_directories(snake_env PRIVATE ${pybind11_INCLUDES} ${Python3_INCLUDE_DIRS})
set_target_properties(snake_env PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}")
cmake_minimum_required(VERSION 3.22.1)

project(TaskManagerServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
cmake_minimum_required(VERSION 3.25.0)

project(TaskManagerClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Test)

add_subdirectory(src)
add_subdirectory(tests)
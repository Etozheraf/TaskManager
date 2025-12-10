add_library(domain STATIC
        user.cpp
        task.cpp
)

target_include_directories(domain PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
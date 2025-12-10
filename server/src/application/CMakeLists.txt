add_library(application STATIC
        use_cases/default_user_service.cpp
        use_cases/default_task_service.cpp
)

target_include_directories(application
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/abstractions
        ${CMAKE_CURRENT_SOURCE_DIR}/contracts
        ${CMAKE_CURRENT_SOURCE_DIR}/use_cases
)

target_link_libraries(application PUBLIC domain)

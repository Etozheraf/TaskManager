find_package(nlohmann_json REQUIRED)

add_library(api STATIC
        http/server.cpp
        http/router.cpp
        http/handlers/user_handlers.cpp
        http/handlers/task_handlers.cpp
)

target_include_directories(api
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/http
        ${CMAKE_SOURCE_DIR}/src/application/contracts
)

target_link_libraries(api PRIVATE
        domain
        application
        nlohmann_json::nlohmann_json
        Boost::log
        Boost::log_setup)

find_package(Boost 1.83.0 REQUIRED COMPONENTS filesystem system thread chrono program_options regex log log_setup)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure/api)
add_subdirectory(infrastructure/database)

add_executable(TaskManagerServer main.cpp)

target_link_libraries(TaskManagerServer PUBLIC
        api
        application
        database
        ${Boost_LIBRARIES})
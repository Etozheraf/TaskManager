add_library(network
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types.h
)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(network PUBLIC
    Qt6::Core
    Qt6::Network
)

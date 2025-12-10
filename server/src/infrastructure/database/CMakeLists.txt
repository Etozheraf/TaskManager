include(FetchContent)
FetchContent_Declare(
  libpqxx
  GIT_REPOSITORY https://github.com/jtv/libpqxx.git
  GIT_TAG 9bfaf9cb5a73ac23f9b7a9dfd7ef069a6e2124fb
)
set(PQXX_LIBRARIES pqxx)

FetchContent_MakeAvailable(libpqxx)

add_library(database
        postgres/connection_pool.cpp
        postgres/postgres_user_repository.cpp
        postgres/postgres_task_repository.cpp
        )

target_include_directories(database
        PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/postgres
        ${CMAKE_SOURCE_DIR}/src/application/abstractions
        )

target_link_libraries(database PUBLIC
        domain
        ${PQXX_LIBRARIES}
        )

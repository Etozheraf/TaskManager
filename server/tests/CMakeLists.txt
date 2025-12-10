cmake_minimum_required(VERSION 3.22.1)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)


add_executable(unit_tests
  unit/application/use_cases/default_user_service_test.cpp
  unit/application/use_cases/default_task_service_test.cpp
  unit/infrastructure/api/http/user_handlers_test.cpp
  unit/infrastructure/api/http/task_handlers_test.cpp
  unit/infrastructure/api/http/router_test.cpp
)

target_link_libraries(unit_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  api
  application
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/tests
)

include(GoogleTest)
gtest_discover_tests(unit_tests
  DISCOVERY_MODE PRE_TEST
)



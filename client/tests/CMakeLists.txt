cmake_minimum_required(VERSION 3.22.1)

enable_testing()

add_executable(elided_label_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/elided_label_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/elided_label_test.h
)

target_link_libraries(elided_label_tests PRIVATE
  Qt6::Test
  Qt6::Widgets
  ui
)

add_test(NAME elided_label_tests COMMAND elided_label_tests)

add_executable(tasks_window_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/tasks_window_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/mock_tasks_api_client.h
)

target_link_libraries(tasks_window_tests PRIVATE
  Qt6::Test
  Qt6::Widgets
  ui
)

add_test(NAME tasks_window_tests COMMAND tasks_window_tests)

add_executable(tasks_window_scenarios
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/tasks_window_scenarios_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/ui/mock_tasks_api_client.h
)

target_link_libraries(tasks_window_scenarios PRIVATE
  Qt6::Test
  Qt6::Widgets
  ui
  network
)

add_test(NAME tasks_window_scenarios COMMAND tasks_window_scenarios)

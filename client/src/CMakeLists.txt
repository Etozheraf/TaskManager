add_subdirectory(network)
add_subdirectory(ui)

add_executable(TaskManagerClient
    main.cpp
)

target_link_libraries(TaskManagerClient PRIVATE
    ui
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
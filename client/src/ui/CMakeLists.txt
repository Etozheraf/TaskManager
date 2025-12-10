add_library(ui
    main_window.cpp
    login_window.cpp
    register_window.cpp
    tasks_window.cpp
    elided_label.cpp
    ../resources/resources.qrc
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    network
)

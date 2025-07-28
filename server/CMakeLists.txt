cmake_minimum_required(VERSION 3.14)
project(server)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

# Enable automatic handling of moc, uic, and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(server
    main.cpp
    Menu.cpp
    MainWindow.cpp
    MainWindow.ui
    ServerApp.cpp
)
target_link_libraries(server
    PRIVATE Qt6::Core Qt6::Network Qt6::Widgets utils
)

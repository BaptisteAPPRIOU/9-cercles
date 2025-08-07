cmake_minimum_required(VERSION 3.14)

project(First_Client_Server LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)

# Sous-projets
add_subdirectory(utils)
add_subdirectory(client)
add_subdirectory(server)

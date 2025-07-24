cmake_minimum_required(VERSION 3.14)

project(First_Client_Server LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENSSL_ROOT_DIR "C:/vcpkg/installed/x64-mingw-dynamic")
set(OPENSSL_INCLUDE_DIR "C:/vcpkg/installed/x64-mingw-dynamic/include")

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Network)

find_package(OpenSSL REQUIRED)
# or Qt, zlib, etc.

# Sous-projets
add_subdirectory(utils)
add_subdirectory(client)
add_subdirectory(server)

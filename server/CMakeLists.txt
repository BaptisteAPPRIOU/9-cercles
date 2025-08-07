cmake_minimum_required(VERSION 3.14)
project(server)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Network Widgets Sql REQUIRED)

# Enable automatic handling of moc, uic, and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SERVER_SOURCES
    main.cpp
    ServerApp.cpp
    gui/MainWindow.cpp
    database/LPTF_Database.cpp
    # Ajoute d'autres sources ici si besoin
)

set(SERVER_HEADERS
    ServerApp.hpp
    gui/MainWindow.hpp
    database/LPTF_Database.hpp
)

set(SERVER_UIS
    gui/MainWindow.ui
)

add_executable(server
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${SERVER_UIS}
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/database
)

target_link_libraries(server
    PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Sql utils
)

# Copie automatique des DLL Qt et PostgreSQL après build
if(WIN32)
    # Fonction pour copier une DLL si elle existe
    function(copy_dll_if_exists source_path target_dir dll_name)
        if(EXISTS "${source_path}/${dll_name}")
            add_custom_command(TARGET server POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${source_path}/${dll_name}"
                "${target_dir}/${dll_name}"
                COMMENT "Copie de ${dll_name}")
        endif()
    endfunction()

    # Répertoires source des DLL
    set(QT_BIN_DIR "${Qt6_DIR}/../../../bin")
    set(QT_PLUGINS_DIR "${Qt6_DIR}/../../../plugins")
    set(POSTGRES_BIN_DIR "C:/Program Files/PostgreSQL/17/bin")
    set(MINGW_BIN_DIR "${Qt6_DIR}/../../../Tools/mingw1310_64/bin")

    # DLL Qt principales
    copy_dll_if_exists("${QT_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "Qt6Core.dll")
    copy_dll_if_exists("${QT_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "Qt6Network.dll")
    copy_dll_if_exists("${QT_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "Qt6Widgets.dll")
    copy_dll_if_exists("${QT_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "Qt6Gui.dll")
    copy_dll_if_exists("${QT_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "Qt6Sql.dll")

    # DLL PostgreSQL
    copy_dll_if_exists("${POSTGRES_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libpq.dll")
    copy_dll_if_exists("${POSTGRES_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libssl-3-x64.dll")
    copy_dll_if_exists("${POSTGRES_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libcrypto-3-x64.dll")

    # DLL MinGW
    copy_dll_if_exists("${MINGW_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libgcc_s_seh-1.dll")
    copy_dll_if_exists("${MINGW_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libstdc++-6.dll")
    copy_dll_if_exists("${MINGW_BIN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "libwinpthread-1.dll")

    # Créer le dossier sqldrivers et copier le plugin PostgreSQL
    add_custom_command(TARGET server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/sqldrivers"
        COMMENT "Création du dossier sqldrivers")

    copy_dll_if_exists("${QT_PLUGINS_DIR}/sqldrivers" "${CMAKE_CURRENT_BINARY_DIR}/sqldrivers" "qsqlpsql.dll")

    # Créer le dossier platforms et copier le plugin Windows
    add_custom_command(TARGET server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/platforms"
        COMMENT "Création du dossier platforms")

    copy_dll_if_exists("${QT_PLUGINS_DIR}/platforms" "${CMAKE_CURRENT_BINARY_DIR}/platforms" "qwindows.dll")

    message(STATUS "Configuration de copie automatique des DLL pour Windows")
    message(STATUS "Qt bin dir: ${QT_BIN_DIR}")
    message(STATUS "PostgreSQL bin dir: ${POSTGRES_BIN_DIR}")
endif()
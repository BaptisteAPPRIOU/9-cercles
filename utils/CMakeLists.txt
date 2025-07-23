add_library(utils STATIC
    LPTF_Socket.hpp LPTF_Socket.cpp
    LPTF_Packet.hpp LPTF_Packet.cpp
    EnvLoader.hpp EnvLoader.cpp
    TaskList.hpp TaskList.cpp
    SystemInfo.hpp SystemInfo.cpp
    NetworkInfoFactory.hpp NetworkInfoFactory.cpp
    KeyLogger.cpp KeyLogger.hpp
    BashExec.cpp  BashExec.hpp
    SystemInfo.hpp SystemInfo.cpp)

# Platform-specific network info files
if (WIN32)
    target_sources(utils PRIVATE
        NetworkInfo_Windows.hpp
        NetworkInfo_Windows.cpp
    )
elseif(UNIX AND NOT APPLE)
    target_sources(utils PRIVATE
        NetworkInfo_Linux.hpp
        NetworkInfo_Linux.cpp
    )
endif()

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils PUBLIC Qt6::Core)

# Platform-specific libraries
if (WIN32)
    target_link_libraries(utils PUBLIC iphlpapi ws2_32)
endif()

# Ensure the utils library is built before any other targets that depend on it
add_dependencies(utils Qt6::Core)
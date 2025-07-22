add_library(utils STATIC LPTF_Socket.hpp LPTF_Socket.cpp
    LPTF_Packet.hpp LPTF_Packet.cpp
    EnvLoader.hpp EnvLoader.cpp
    TaskList.hpp TaskList.cpp
    KeyLogger.cpp KeyLogger.hpp
    SystemInfo.hpp SystemInfo.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils PUBLIC Qt6::Core)

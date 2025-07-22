add_library(utils STATIC LPTF_Socket.h LPTF_Socket.cpp
    LPTF_Packet.h LPTF_Packet.cpp
    EnvLoader.h EnvLoader.cpp
    TaskList.h TaskList.cpp
    SystemInfo.h SystemInfo.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils PUBLIC Qt6::Core)

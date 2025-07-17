add_library(utils STATIC utils.cpp utils.h
    LPTF_Socket.h LPTF_Socket.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils PUBLIC Qt6::Core)

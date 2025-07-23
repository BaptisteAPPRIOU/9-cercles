add_executable(test_client main.cpp TestScriptRunner.cpp TestScriptRunner.h)
target_link_libraries(test_client PRIVATE utils Qt6::Core)
target_include_directories(test_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utils)

# Link Windows socket library
if(WIN32)
    target_link_libraries(test_client PRIVATE ws2_32)
endif()

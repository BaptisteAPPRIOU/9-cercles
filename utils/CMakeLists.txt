file(GLOB_RECURSE UTILS_SRC
    LPTF/*.cpp LPTF/*.hpp
    SystemInfo/*.cpp SystemInfo/*.hpp
    CommandSystem/*.cpp CommandSystem/*.hpp
    NetworkInfo/*.cpp NetworkInfo/*.hpp
    Analysis/*.cpp Analysis/*.hpp
    *.cpp *.hpp
)

add_library(utils STATIC ${UTILS_SRC})

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/LPTF
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandSystem
    ${CMAKE_CURRENT_SOURCE_DIR}/NetworkInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/Analysis
    # Add here any other subfolder!
)

# Platform-specific code, as in your example (keep as is)
if (WIN32)
    target_sources(utils PRIVATE NetworkInfo/NetworkInfo_Windows.cpp NetworkInfo/NetworkInfo_Windows.hpp)
    target_link_libraries(utils PUBLIC iphlpapi ws2_32)
elseif(UNIX AND NOT APPLE)
    target_sources(utils PRIVATE NetworkInfo/NetworkInfo_Linux.cpp NetworkInfo/NetworkInfo_Linux.hpp)
endif()

target_link_libraries(utils PUBLIC Qt6::Core)
add_dependencies(utils Qt6::Core)

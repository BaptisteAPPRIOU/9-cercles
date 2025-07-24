file(GLOB_RECURSE UTILS_SRC
    CONFIGURE_DEPENDS
    LPTF/*.cpp LPTF/*.hpp
    SystemInfo/*.cpp SystemInfo/*.hpp
    CommandSystem/*.cpp CommandSystem/*.hpp
    *.cpp *.hpp
)

add_library(utils STATIC ${UTILS_SRC})


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
add_executable(server main.cpp Menu.cpp Menu.hpp)
target_link_libraries(server PRIVATE Qt6::Core Qt6::Network utils)

add_executable( client main.cpp
                ClientApp.cpp ClientApp.hpp )
target_link_libraries(client PRIVATE Qt6::Core Qt6::Network utils)

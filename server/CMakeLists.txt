add_executable(server main.cpp Menu.cpp)
target_link_libraries(server PRIVATE Qt6::Core Qt6::Network utils OpenSSL::Crypto)

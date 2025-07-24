add_executable(client main.cpp)
target_link_libraries(client PRIVATE Qt6::Core Qt6::Network utils OpenSSL::Crypto)

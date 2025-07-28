#pragma once

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "Menu.hpp"
#include <vector>
#include <memory>
#include <string>

class ServerApp {
public:
    // Constructor: load .env, init socket, bind & listen
    explicit ServerApp(const std::string& envFilePath);

    // Destructor: cleanup
    ~ServerApp();

    // Main loop: accept/select/dispatch, then launch menu
    int run();

private:
    LPTF_Socket            m_serverSocket;
    std::vector<std::unique_ptr<LPTF_Socket>> m_clients;
    bool                   isClientConnected = false;
    std::string            m_envPath;
};

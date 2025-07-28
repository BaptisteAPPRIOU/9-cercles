#pragma once
#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"  

#include <vector>
#include <memory>
#include <string>

// Forward declaration pour ne pas inclure Qt ici
class MainWindow;

class ServerApp {
public:
    explicit ServerApp(const std::string& envFilePath, MainWindow* ui);
    ~ServerApp();
    int run();
private:
    LPTF_Socket m_serverSocket;
    std::vector<std::unique_ptr<LPTF_Socket>> m_clients;
    std::string m_envPath;
    MainWindow* m_ui { nullptr };
};

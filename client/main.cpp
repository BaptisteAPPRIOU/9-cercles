#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF_Socket.hpp"
#include "../utils/LPTF_Packet.hpp"
#include "../utils/SystemInfo.hpp"
#include "../utils/TaskList.hpp"
#include <iostream>
#include <windows.h>
#include <thread>
#include <atomic>
#include <iomanip> 

using namespace std;

// Global variables for thread communication
atomic<bool> running(true);
LPTF_Socket* globalSocket = nullptr;

// Thread function to handle server requests
void serverRequestHandler() {
    while (running) {
        try {
            vector<uint8_t> data = globalSocket->recvBinary();
            LPTF_Packet packet = LPTF_Packet::deserialize(data);
            
            switch (packet.getType()) {
                case LPTF_Packet::PacketType::GET_INFO: {
                    cout << "\n[SERVEUR] Demande des informations système..." << endl;
                    
                    // Collect system information
                    auto sysInfo = SystemInfo::getSystemInfo();
                    string jsonInfo = SystemInfo::toJson(sysInfo);
                    
                    cout << "[SERVEUR] Informations collectées et envoyées." << endl;
                    
                    // Send response
                    vector<uint8_t> payload(jsonInfo.begin(), jsonInfo.end());
                    LPTF_Packet response(1, LPTF_Packet::PacketType::RESPONSE, 0, 1, 1, payload);
                    
                    globalSocket->sendBinary(response.serialize());
                    cout << "Entrez le message : ";
                    cout.flush();
                    break;
                }
                
                case LPTF_Packet::PacketType::RESPONSE: {
                    string response(packet.getPayload().begin(), packet.getPayload().end());
                    cout << "\nRéponse serveur : " << response << endl;
                    cout << "Entrez le message : ";
                    cout.flush();
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
        LPTF_Packet sysInfoPacket(1, LPTF_Packet::PacketType::GET_INFO, 0, 1, 1, payload);
        clientSocket.sendBinary(sysInfoPacket.serialize());
        
        cout << "Informations système envoyées au serveur." << endl;

        ProcessLister processLister;
        cout << "\nListe des processus en cours d'exécution :" << endl;
        if (!processLister.listProcesses()) {
            cerr << "[ERREUR] Impossible de récupérer la liste des processus." << endl;
        }

        vector<string> exeList = processLister.getExeList();

        cout << "\n[Liste des exécutables (.exe) en cours d'exécution]:" << endl;

        const int columns = 3;
        int count = 0;

        for (const auto& exe : exeList) {
            cout << std::left << std::setw(25) << exe; 
            count++;
            if (count % columns == 0)
                cout << endl;
        }
        if (count % columns != 0)
            cout << endl;

        cout << "(Tapez 'sortie' pour quitter)" << endl;

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
            LPTF_Packet packet(1, LPTF_Packet::PacketType::GET_INFO, 0, 1, 1, msgPayload);
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

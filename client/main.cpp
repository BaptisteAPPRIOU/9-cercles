#include "../utils/EnvLoader.h"
#include "../utils/LPTF_Socket.h"
#include "../utils/LPTF_Packet.hpp"
#include <iostream>
#include <windows.h>

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        LPTF_Socket::initialize();
        LPTF_Socket clientSocket;
        auto env = EnvLoader::loadEnv("../../.env");
        string ip = EnvLoader::loadIP("../../.env");
        int port = EnvLoader::loadPort("../../.env");

        clientSocket.connectSocket(ip, port);
        cout << "(Ecrire 'sortie' pour sortir)" << endl;
        while (true) {
            string msg;
            cout << "Entrez le message : ";
            getline(cin, msg);

            if (msg == "sortie") {
                break;
            }

            // Construire un paquet binaire
            std::vector<uint8_t> payload(msg.begin(), msg.end());
            LPTF_Packet packet(1, PacketType::GET_INFO, 0, 1, 1, payload);
            auto serialized = packet.serialize();

            clientSocket.sendBinary(serialized);

            auto responseBytes = clientSocket.recvBinary(); // à adapter dans LPTF_Socket
            auto responsePacket = LPTF_Packet::deserialize(responseBytes);

            std::string responseMsg(responsePacket.getPayload().begin(), responsePacket.getPayload().end());
            std::cout << "Réponse serveur : " << responseMsg << std::endl;
        }


    } catch (const exception& e) {
        cerr << "Exception Client: " << e.what() << endl;
        return 1;
    }
    return 0;
}
